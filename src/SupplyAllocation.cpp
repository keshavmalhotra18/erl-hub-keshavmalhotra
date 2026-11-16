#include "SupplyAllocation.hpp"

#include <algorithm>
#include <cmath>

#include "Exceptions.hpp"
#include "ReliefVehicle.hpp"

namespace ReliefOps {
namespace Logistics {

using Inventory::DistributionDepot;
using Inventory::ReliefCargo;

namespace {
ReliefCargo withQuantity(const ReliefCargo& src, int qty) {
    return ReliefCargo(src.id(), src.category(), qty, src.unitWeightKg(), src.unitVolumeM3(),
                       src.triagePriority());
}
}  // namespace

ReliefCargo allocateSupply(const DistributionDepot& depot, const std::string& itemId, double volumeM3) {
    const ReliefCargo& stock = depot.item(itemId);
    const int fit = stock.unitVolumeM3() > 0.0 ? static_cast<int>(std::floor(volumeM3 / stock.unitVolumeM3()))
                                                : stock.quantity();
    return withQuantity(stock, std::max(0, std::min(fit, stock.quantity())));
}

ReliefCargo allocateSupply(const DistributionDepot& depot, const std::string& itemId, Triage level,
                           int patients) {
    const ReliefCargo& stock = depot.item(itemId);
    const int perPatient = level == Triage::Critical ? 3 : (level == Triage::Urgent ? 2 : 1);
    return withQuantity(stock, std::min(stock.quantity(), std::max(0, patients) * perPatient));
}

ReliefCargo allocateSupply(const DistributionDepot& depot, const std::string& itemId,
                           MissionParameters& mission) {
    const ReliefCargo& stock = depot.item(itemId);
    const int affordable = stock.unitWeightKg() > 0.0
                               ? static_cast<int>(std::floor(mission.payloadBudgetKg / stock.unitWeightKg()))
                               : stock.quantity();
    ReliefCargo batch = withQuantity(stock, std::max(0, std::min(affordable, stock.quantity())));
    mission.payloadBudgetKg -= batch.totalWeightKg();  // updated through the reference
    return batch;
}

MissionParameters& highestPriorityMission(std::vector<MissionParameters>& missions) {
    if (missions.empty()) throw ErlException("highestPriorityMission: no missions queued");
    return *std::max_element(missions.begin(), missions.end(),
                             [](const MissionParameters& a, const MissionParameters& b) {
                                 return a.priority < b.priority;
                             });
}

void dispatchToVehicle(DistributionDepot& depot, ReliefVehicle& vehicle, const ReliefCargo& batch) {
    vehicle.load(batch);  // may throw CargoOverweightException BEFORE depot is touched
    depot -= batch;       // depot deducts upon departure and records dispatched tonnage
}

}  // namespace Logistics
}  // namespace ReliefOps
