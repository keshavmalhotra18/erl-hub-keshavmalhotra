// Unit I: friend class that may bypass access barriers during critical triage overrides.
#pragma once

#include <string>

#include "DistributionDepot.hpp"
#include "ReliefCargo.hpp"
#include "ReliefVehicle.hpp"

namespace ReliefOps {

class IncidentCommander {
public:
    explicit IncidentCommander(std::string callsign) : callsign_(std::move(callsign)) {}

    // Raise/lower a cargo's triage level (normally immutable).
    void overrideTriage(Inventory::ReliefCargo& cargo, int level) const;
    // Emergency fuel injection beyond normal refuelling rules.
    void emergencyRefuel(Logistics::ReliefVehicle& v, double litres) const;
    // Lift a vehicle's payload ceiling (e.g. one-way evacuation sortie).
    void overridePayloadLimit(Logistics::ReliefVehicle& v, double newLimitKg) const;
    // Expand depot capacity (tent/overflow storage).
    void expandDepotCapacity(Inventory::DistributionDepot& d, double extraM3) const;

    const std::string& callsign() const noexcept { return callsign_; }

private:
    std::string callsign_;
};

}  // namespace ReliefOps
