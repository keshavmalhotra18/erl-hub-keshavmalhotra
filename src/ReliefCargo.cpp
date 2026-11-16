#include "ReliefCargo.hpp"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>

#include "Exceptions.hpp"

namespace ReliefOps {
namespace Inventory {

double ReliefCargo::totalDispatchedKg_ = 0.0;

const char* toString(Category c) noexcept {
    switch (c) {
        case Category::RationKit: return "Ration Kit";
        case Category::TraumaMedicine: return "Trauma Medicine";
        case Category::CleanWater: return "Clean Water";
        case Category::ThermalBlanket: return "Thermal Blanket";
    }
    return "Unknown";
}

ReliefCargo::ReliefCargo()
    : id_(), category_(Category::RationKit), quantity_(0), unitWeightKg_(0.0), unitVolumeM3_(0.0), triage_(1) {}

ReliefCargo::ReliefCargo(std::string id, Category category, int quantity, double unitWeightKg,
                         double unitVolumeM3, int triagePriority)
    : id_(std::move(id)),
      category_(category),
      quantity_(quantity),
      unitWeightKg_(unitWeightKg),
      unitVolumeM3_(unitVolumeM3),
      triage_(std::clamp(triagePriority, 1, 5)) {
    if (quantity < 0 || unitWeightKg < 0.0 || unitVolumeM3 < 0.0)
        throw ErlException("ReliefCargo: negative quantity/weight/volume for " + id_);
}

ReliefCargo& ReliefCargo::operator+=(int units) {
    if (units < 0) throw ErlException("ReliefCargo::operator+=: negative restock for " + id_);
    quantity_ += units;
    return *this;
}

ReliefCargo& ReliefCargo::operator-=(int units) {
    if (units < 0) throw ErlException("ReliefCargo::operator-=: negative deduction for " + id_);
    if (units > quantity_)
        throw InsufficientStockException(id_ + " requested " + std::to_string(units) + ", have " +
                                         std::to_string(quantity_));
    quantity_ -= units;
    return *this;
}

bool ReliefCargo::operator==(const ReliefCargo& rhs) const noexcept {
    return id_ == rhs.id_ && category_ == rhs.category_;
}

// Serialization format (whitespace separated, one line):
//   <id> <categoryIndex> <quantity> <unitWeightKg> <unitVolumeM3> <triage>
std::ostream& operator<<(std::ostream& os, const ReliefCargo& c) {
    const auto oldPrecision = os.precision(std::numeric_limits<double>::max_digits10);
    os << c.id_ << ' ' << static_cast<int>(c.category_) << ' ' << c.quantity_ << ' ' << c.unitWeightKg_
       << ' ' << c.unitVolumeM3_ << ' ' << c.triage_;
    os.precision(oldPrecision);
    return os;
}

std::istream& operator>>(std::istream& is, ReliefCargo& c) {
    std::string id;
    int cat = 0, qty = 0, triage = 1;
    double w = 0.0, v = 0.0;
    if (!(is >> id >> cat >> qty >> w >> v >> triage)) return is;
    if (cat < 0 || cat > 3 || qty < 0 || w < 0.0 || v < 0.0) {
        is.setstate(std::ios::failbit);
        return is;
    }
    c = ReliefCargo(std::move(id), static_cast<Category>(cat), qty, w, v, triage);
    return is;
}

}  // namespace Inventory
}  // namespace ReliefOps
