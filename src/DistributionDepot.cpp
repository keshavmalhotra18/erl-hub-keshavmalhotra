#include "DistributionDepot.hpp"

#include <algorithm>

#include "Exceptions.hpp"

namespace ReliefOps {
namespace Inventory {

DistributionDepot::DistributionDepot(std::string name, double capacityM3)
    : name_(std::move(name)), capacityM3_(capacityM3), stock_() {}

double DistributionDepot::usedVolumeM3() const {
    double total = 0.0;
    for (const auto& kv : stock_) total += kv.second.totalVolumeM3();
    return total;
}

ReliefCargo& DistributionDepot::item(const std::string& itemId) {
    auto it = stock_.find(itemId);
    if (it == stock_.end()) throw InsufficientStockException(itemId + " not stocked at " + name_);
    return it->second;
}

const ReliefCargo& DistributionDepot::item(const std::string& itemId) const {
    auto it = stock_.find(itemId);
    if (it == stock_.end()) throw InsufficientStockException(itemId + " not stocked at " + name_);
    return it->second;
}

DistributionDepot& DistributionDepot::operator+=(const ReliefCargo& cargo) {
    if (usedVolumeM3() + cargo.totalVolumeM3() > capacityM3_ + 1e-9) throw DepotCapacityException(name_);
    auto it = stock_.find(cargo.id());
    if (it == stock_.end())
        stock_.emplace(cargo.id(), cargo);
    else
        it->second += cargo.quantity();
    return *this;
}

DistributionDepot& DistributionDepot::operator-=(const ReliefCargo& cargo) {
    auto it = stock_.find(cargo.id());
    if (it == stock_.end()) throw InsufficientStockException(cargo.id() + " not stocked at " + name_);
    it->second -= cargo.quantity();  // may throw
    ReliefCargo::recordDispatch(cargo.unitWeightKg() * cargo.quantity());
    if (it->second.quantity() == 0) stock_.erase(it);
    return *this;
}

std::vector<ReliefCargo> DistributionDepot::snapshot() const {
    std::vector<ReliefCargo> out;
    out.reserve(stock_.size());
    for (const auto& kv : stock_) out.push_back(kv.second);
    std::sort(out.begin(), out.end(),
              [](const ReliefCargo& a, const ReliefCargo& b) { return a.id() < b.id(); });
    return out;
}

}  // namespace Inventory
}  // namespace ReliefOps
