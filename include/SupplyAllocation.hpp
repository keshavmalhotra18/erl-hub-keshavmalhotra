// Unit I: function overloading (volume-based vs. triage-based allocation),
// call by reference and return by reference.
#pragma once

#include <string>
#include <vector>

#include "DistributionDepot.hpp"
#include "ReliefCargo.hpp"

namespace ReliefOps {
namespace Logistics {

enum class Triage : int { Routine = 1, Urgent = 3, Critical = 5 };

struct MissionParameters {
    std::string name;
    double payloadBudgetKg = 0.0;  // remaining payload budget for the mission
    int priority = 1;              // 1..5
    int population = 0;            // people served
};

// (1) Volume-based allocation: how many units of `itemId` fit in `volumeM3`.
Inventory::ReliefCargo allocateSupply(const Inventory::DistributionDepot& depot, const std::string& itemId,
                                      double volumeM3);

// (2) Priority-based emergency medical triage allocation: units per patient
//     scale with the triage level (Critical x3, Urgent x2, Routine x1).
Inventory::ReliefCargo allocateSupply(const Inventory::DistributionDepot& depot, const std::string& itemId,
                                      Triage level, int patients);

// (3) Mission-based allocation. `mission` is passed BY REFERENCE and its
//     remaining payload budget is updated in place (no copying of the struct).
Inventory::ReliefCargo allocateSupply(const Inventory::DistributionDepot& depot, const std::string& itemId,
                                      MissionParameters& mission);

// Return by reference: the highest-priority mission, usable as an lvalue.
MissionParameters& highestPriorityMission(std::vector<MissionParameters>& missions);

// Call by reference: reduces the depot record in place and loads the vehicle.
class ReliefVehicle;
void dispatchToVehicle(Inventory::DistributionDepot& depot, ReliefVehicle& vehicle,
                       const Inventory::ReliefCargo& batch);

}  // namespace Logistics
}  // namespace ReliefOps
