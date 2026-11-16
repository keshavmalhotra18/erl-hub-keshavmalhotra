// Unit II / IV: DistributionDepot - stock held in std::unordered_map, with
// operator+= (restock) and operator-= (deduct on vehicle departure).
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Forward.hpp"
#include "ReliefCargo.hpp"

namespace ReliefOps {
namespace Inventory {

class DistributionDepot {
public:
    DistributionDepot(std::string name, double capacityM3);

    const std::string& name() const noexcept { return name_; }
    double capacityM3() const noexcept { return capacityM3_; }
    double usedVolumeM3() const;
    bool has(const std::string& itemId) const { return stock_.count(itemId) != 0; }

    // Unit I: return by reference (caller may update the stock record in place).
    ReliefCargo& item(const std::string& itemId);
    const ReliefCargo& item(const std::string& itemId) const;

    // Restock: merges into an existing record or creates one. Throws DepotCapacityException.
    DistributionDepot& operator+=(const ReliefCargo& cargo);
    // Deduct upon vehicle departure. Throws InsufficientStockException. Updates dispatched tonnage.
    DistributionDepot& operator-=(const ReliefCargo& cargo);

    std::vector<ReliefCargo> snapshot() const;  // sorted by id

    friend class ReliefOps::IncidentCommander;

private:
    std::string name_;
    double capacityM3_;
    std::unordered_map<std::string, ReliefCargo> stock_;
};

}  // namespace Inventory
}  // namespace ReliefOps
