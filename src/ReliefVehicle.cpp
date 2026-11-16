#include "ReliefVehicle.hpp"

#include <iostream>

#include "Exceptions.hpp"
#include "GeoUtils.hpp"

namespace ReliefOps {
namespace Logistics {

int ReliefVehicle::operational_ = 0;
bool ReliefVehicle::verbose_ = false;

ReliefVehicle::ReliefVehicle(std::string id, double maxPayloadKg, double fuelL, double burnLPerKm)
    : id_(id), hold_(id, maxPayloadKg), gps_(), lifeSupport_(), fuelL_(fuelL), burnLPerKm_(burnLPerKm) {
    ++operational_;
}

ReliefVehicle::~ReliefVehicle() {
    --operational_;
    announce("~ReliefVehicle", id_);
}

void ReliefVehicle::announce(const char* what, const std::string& id) {
    if (verbose_) std::cout << "    [lifecycle] " << what << " released (" << id << ")\n";
}

void ReliefVehicle::requireFuel(const RouteSegment& seg) const {
    const double need = fuelNeededL(seg);
    if (need > fuelL_) throw FuelDepletionException(need, fuelL_);
}

double ReliefVehicle::travel(const RouteSegment& seg) {
    const double eta = evaluateRoute(seg);  // virtual dispatch; may throw before any state changes
    fuelL_ -= fuelNeededL(seg);
    gps_.update(seg.endX, seg.endY, seg.name);
    lifeSupport_.consumeOxygen(static_cast<int>(eta * 60.0));
    return eta;
}

void ReliefVehicle::refuel(double litres) {
    if (litres > 0.0) fuelL_ += litres;
}

// ---------------- AerialTransport ----------------
AerialTransport::AerialTransport(std::string id) : ReliefVehicle(std::move(id), 1500.0, 900.0, 4.0) {}
AerialTransport::~AerialTransport() { announce("~AerialTransport", this->id()); }

double AerialTransport::evaluateRoute(const RouteSegment& seg) const {
    if (seg.airspaceClosed) throw DamagedCorridorException(seg.name + " (airspace closed)");
    requireFuel(seg);
    return etaHours(seg.distanceKm, 180.0);  // flies over rubble and water
}

// ---------------- GroundConvoy ----------------
GroundConvoy::GroundConvoy(std::string id) : ReliefVehicle(std::move(id), 8000.0, 450.0, 0.6) {}
GroundConvoy::~GroundConvoy() { announce("~GroundConvoy", this->id()); }

double GroundConvoy::evaluateRoute(const RouteSegment& seg) const {
    if (seg.corridorDamaged || seg.waterCrossing) throw DamagedCorridorException(seg.name);
    requireFuel(seg);
    return etaHours(seg.distanceKm, 50.0 * (1.0 - 0.6 * seg.roughness));
}

// ---------------- MarineVessel ----------------
MarineVessel::MarineVessel(std::string id) : ReliefVehicle(std::move(id), 3000.0, 300.0, 1.2) {}
MarineVessel::~MarineVessel() { announce("~MarineVessel", this->id()); }

double MarineVessel::evaluateRoute(const RouteSegment& seg) const {
    if (!seg.waterCrossing) throw DamagedCorridorException(seg.name + " (no navigable water)");
    requireFuel(seg);
    return etaHours(seg.distanceKm, 28.0);
}

// ---------------- AmphibiousCraft ----------------
// The most-derived class initialises the single virtual base directly.
AmphibiousCraft::AmphibiousCraft(std::string id)
    : ReliefVehicle(id, 4000.0, 350.0, 1.0), GroundConvoy(id), MarineVessel(id) {
    lifeSupport().activate();  // mobile medical unit
}
AmphibiousCraft::~AmphibiousCraft() { announce("~AmphibiousCraft", this->id()); }

double AmphibiousCraft::evaluateRoute(const RouteSegment& seg) const {
    // Water legs behave like a boat; land legs may be rough but a damaged corridor
    // is still traversable amphibiously only if flooded (waterCrossing).
    if (seg.waterCrossing) return MarineVessel::evaluateRoute(seg);
    return GroundConvoy::evaluateRoute(seg);
}

}  // namespace Logistics
}  // namespace ReliefOps
