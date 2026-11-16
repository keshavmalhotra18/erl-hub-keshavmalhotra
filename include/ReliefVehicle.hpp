// Unit III: abstract base class ReliefVehicle and its hierarchy.
//
//                 ReliefVehicle  (abstract, virtual base)
//                 /     |      \
//      AerialTransport  GroundConvoy  MarineVessel
//                           \          /
//                          AmphibiousCraft      <- diamond resolved by virtual inheritance
//
// GPSBeacon / EmergencyLifeSupportUnit / TransportVehicle are COMPOSED
// (containership) in ReliefVehicle rather than inherited.
#pragma once

#include <string>

#include "Components.hpp"
#include "Forward.hpp"
#include "ReliefCargo.hpp"
#include "TransportVehicle.hpp"

namespace ReliefOps {
namespace Logistics {

struct RouteSegment {
    std::string name;
    double distanceKm = 0.0;
    double roughness = 0.0;          // 0 (clear) .. 1 (rubble)
    bool corridorDamaged = false;    // road/bridge destroyed
    bool waterCrossing = false;      // flooded / river / coast leg
    bool airspaceClosed = false;     // storm cell or no-fly restriction
    double endX = 0.0, endY = 0.0;   // grid position reached
};

class ReliefVehicle {
public:
    virtual ~ReliefVehicle();
    ReliefVehicle(const ReliefVehicle&) = delete;
    ReliefVehicle& operator=(const ReliefVehicle&) = delete;

    // Runtime polymorphism: ETA in hours, or throws
    // DamagedCorridorException / FuelDepletionException.
    virtual double evaluateRoute(const RouteSegment& seg) const = 0;
    virtual std::string kind() const = 0;

    const std::string& id() const noexcept { return id_; }
    double fuelL() const noexcept { return fuelL_; }
    const TransportVehicle& hold() const noexcept { return hold_; }
    const GPSBeacon& gps() const noexcept { return gps_; }
    const EmergencyLifeSupportUnit& lifeSupport() const noexcept { return lifeSupport_; }
    EmergencyLifeSupportUnit& lifeSupport() noexcept { return lifeSupport_; }

    void load(const Inventory::ReliefCargo& cargo) { hold_ += cargo; }
    void unload(const Inventory::ReliefCargo& cargo) { hold_ -= cargo; }
    double travel(const RouteSegment& seg);  // evaluates, burns fuel, updates GPS
    void refuel(double litres);

    // Static analytics
    static int totalOperationalVehicles() noexcept { return operational_; }
    static void setVerboseLifecycle(bool on) noexcept { verbose_ = on; }

    friend class ReliefOps::IncidentCommander;

protected:
    ReliefVehicle(std::string id, double maxPayloadKg, double fuelL, double burnLPerKm);
    double fuelNeededL(const RouteSegment& seg) const noexcept { return seg.distanceKm * burnLPerKm_; }
    void requireFuel(const RouteSegment& seg) const;  // throws FuelDepletionException
    static void announce(const char* what, const std::string& id);

private:
    std::string id_;
    TransportVehicle hold_;                 // containership
    GPSBeacon gps_;                         // containership
    EmergencyLifeSupportUnit lifeSupport_;  // containership
    double fuelL_;
    double burnLPerKm_;

    static int operational_;
    static bool verbose_;
};

class AerialTransport : public virtual ReliefVehicle {
public:
    explicit AerialTransport(std::string id);
    ~AerialTransport() override;
    double evaluateRoute(const RouteSegment& seg) const override;
    std::string kind() const override { return "AerialTransport"; }
};

class GroundConvoy : public virtual ReliefVehicle {
public:
    explicit GroundConvoy(std::string id);
    ~GroundConvoy() override;
    double evaluateRoute(const RouteSegment& seg) const override;
    std::string kind() const override { return "GroundConvoy"; }
};

class MarineVessel : public virtual ReliefVehicle {
public:
    explicit MarineVessel(std::string id);
    ~MarineVessel() override;
    double evaluateRoute(const RouteSegment& seg) const override;
    std::string kind() const override { return "MarineVessel"; }
};

// Hybrid amphibious mobile medical unit: inherits GroundConvoy AND MarineVessel,
// which both virtually inherit ReliefVehicle -> exactly ONE ReliefVehicle subobject.
class AmphibiousCraft : public GroundConvoy, public MarineVessel {
public:
    explicit AmphibiousCraft(std::string id);
    ~AmphibiousCraft() override;
    double evaluateRoute(const RouteSegment& seg) const override;
    std::string kind() const override { return "AmphibiousCraft"; }
};

}  // namespace Logistics
}  // namespace ReliefOps
