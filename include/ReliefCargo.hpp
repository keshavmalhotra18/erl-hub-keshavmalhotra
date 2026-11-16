// Unit II: ReliefCargo - abstract data type with information hiding,
// static analytics and operator overloading (+=, -=, ==, <<, >>).
#pragma once

#include <iosfwd>
#include <string>

#include "Forward.hpp"

namespace ReliefOps {
namespace Inventory {

enum class Category : int { RationKit = 0, TraumaMedicine = 1, CleanWater = 2, ThermalBlanket = 3 };

const char* toString(Category c) noexcept;

class ReliefCargo {
public:
    ReliefCargo();
    ReliefCargo(std::string id, Category category, int quantity, double unitWeightKg,
                double unitVolumeM3, int triagePriority = 1);

    // --- read-only interface (data hiding) ---
    const std::string& id() const noexcept { return id_; }
    Category category() const noexcept { return category_; }
    int quantity() const noexcept { return quantity_; }
    double unitWeightKg() const noexcept { return unitWeightKg_; }
    double unitVolumeM3() const noexcept { return unitVolumeM3_; }
    int triagePriority() const noexcept { return triage_; }  // 1 (routine) .. 5 (critical)
    double totalWeightKg() const noexcept { return unitWeightKg_ * quantity_; }
    double totalVolumeM3() const noexcept { return unitVolumeM3_ * quantity_; }

    // --- operator overloading ---
    ReliefCargo& operator+=(int units);        // restock
    ReliefCargo& operator-=(int units);        // deduct (throws InsufficientStockException)
    bool operator==(const ReliefCargo& rhs) const noexcept;  // identity: same id + category
    bool operator!=(const ReliefCargo& rhs) const noexcept { return !(*this == rhs); }

    friend std::ostream& operator<<(std::ostream& os, const ReliefCargo& c);
    friend std::istream& operator>>(std::istream& is, ReliefCargo& c);

    // --- static logistics analytics ---
    static double totalDispatchedTonnes() noexcept { return totalDispatchedKg_ / 1000.0; }
    static void recordDispatch(double kg) noexcept { totalDispatchedKg_ += kg; }
    static void resetStatistics() noexcept { totalDispatchedKg_ = 0.0; }

    friend class ReliefOps::IncidentCommander;  // Unit I: friend class for triage overrides

private:
    std::string id_;
    Category category_;
    int quantity_;
    double unitWeightKg_;
    double unitVolumeM3_;
    int triage_;

    static double totalDispatchedKg_;
};

}  // namespace Inventory
}  // namespace ReliefOps
