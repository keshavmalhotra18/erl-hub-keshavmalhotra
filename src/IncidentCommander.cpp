#include "IncidentCommander.hpp"

#include <algorithm>

namespace ReliefOps {

void IncidentCommander::overrideTriage(Inventory::ReliefCargo& cargo, int level) const {
    cargo.triage_ = std::clamp(level, 1, 5);  // private member, reachable through friendship
}

void IncidentCommander::emergencyRefuel(Logistics::ReliefVehicle& v, double litres) const {
    if (litres > 0.0) v.fuelL_ += litres;
}

void IncidentCommander::overridePayloadLimit(Logistics::ReliefVehicle& v, double newLimitKg) const {
    v.hold_.maxPayloadKg_ = newLimitKg;
}

void IncidentCommander::expandDepotCapacity(Inventory::DistributionDepot& d, double extraM3) const {
    if (extraM3 > 0.0) d.capacityM3_ += extraM3;
}

}  // namespace ReliefOps
