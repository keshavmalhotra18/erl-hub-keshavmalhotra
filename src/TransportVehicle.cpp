#include "TransportVehicle.hpp"

#include <istream>
#include <limits>
#include <ostream>
#include <utility>

#include "Exceptions.hpp"

namespace ReliefOps {
namespace Logistics {

using Inventory::ReliefCargo;

TransportVehicle::TransportVehicle() : TransportVehicle("UNASSIGNED", 0.0, 4) {}

TransportVehicle::TransportVehicle(std::string id, double maxPayloadKg, std::size_t initialSlots)
    : id_(std::move(id)),
      maxPayloadKg_(maxPayloadKg),
      manifest_(initialSlots ? new ReliefCargo[initialSlots] : nullptr),
      count_(0),
      capacity_(initialSlots) {}

TransportVehicle::TransportVehicle(const TransportVehicle& other)
    : id_(other.id_),
      maxPayloadKg_(other.maxPayloadKg_),
      manifest_(other.capacity_ ? new ReliefCargo[other.capacity_] : nullptr),
      count_(other.count_),
      capacity_(other.capacity_) {
    for (std::size_t i = 0; i < count_; ++i) manifest_[i] = other.manifest_[i];  // deep copy
}

TransportVehicle::TransportVehicle(TransportVehicle&& other) noexcept
    : id_(std::move(other.id_)),
      maxPayloadKg_(other.maxPayloadKg_),
      manifest_(other.manifest_),
      count_(other.count_),
      capacity_(other.capacity_) {
    other.manifest_ = nullptr;
    other.count_ = other.capacity_ = 0;
}

TransportVehicle& TransportVehicle::operator=(TransportVehicle other) noexcept {
    swap(*this, other);
    return *this;
}

TransportVehicle::~TransportVehicle() { delete[] manifest_; }

void swap(TransportVehicle& a, TransportVehicle& b) noexcept {
    using std::swap;
    swap(a.id_, b.id_);
    swap(a.maxPayloadKg_, b.maxPayloadKg_);
    swap(a.manifest_, b.manifest_);
    swap(a.count_, b.count_);
    swap(a.capacity_, b.capacity_);
}

double TransportVehicle::loadedKg() const noexcept {
    double total = 0.0;
    for (std::size_t i = 0; i < count_; ++i) total += manifest_[i].totalWeightKg();
    return total;
}

const ReliefCargo& TransportVehicle::at(std::size_t i) const {
    if (i >= count_) throw ErlException("TransportVehicle::at: index out of range");
    return manifest_[i];
}

void TransportVehicle::grow() {
    const std::size_t newCap = capacity_ ? capacity_ * 2 : 4;
    ReliefCargo* bigger = new ReliefCargo[newCap];
    for (std::size_t i = 0; i < count_; ++i) bigger[i] = manifest_[i];
    delete[] manifest_;
    manifest_ = bigger;
    capacity_ = newCap;
}

int TransportVehicle::indexOf(const ReliefCargo& cargo) const noexcept {
    for (std::size_t i = 0; i < count_; ++i)
        if (manifest_[i] == cargo) return static_cast<int>(i);
    return -1;
}

void TransportVehicle::removeAt(std::size_t index) noexcept {
    for (std::size_t i = index; i + 1 < count_; ++i) manifest_[i] = manifest_[i + 1];
    --count_;
}

TransportVehicle& TransportVehicle::operator+=(const ReliefCargo& cargo) {
    const double projected = loadedKg() + cargo.totalWeightKg();
    if (projected > maxPayloadKg_ + 1e-9) throw CargoOverweightException(projected - maxPayloadKg_);
    const int idx = indexOf(cargo);
    if (idx >= 0) {
        manifest_[idx] += cargo.quantity();
    } else {
        if (count_ == capacity_) grow();
        manifest_[count_++] = cargo;
    }
    return *this;
}

TransportVehicle& TransportVehicle::operator-=(const ReliefCargo& cargo) {
    const int idx = indexOf(cargo);
    if (idx < 0) throw InsufficientStockException(cargo.id() + " not aboard " + id_);
    manifest_[idx] -= cargo.quantity();  // may throw
    if (manifest_[idx].quantity() == 0) removeAt(static_cast<std::size_t>(idx));
    return *this;
}

bool TransportVehicle::operator==(const TransportVehicle& rhs) const noexcept {
    if (count_ != rhs.count_) return false;
    for (std::size_t i = 0; i < count_; ++i) {
        const int j = rhs.indexOf(manifest_[i]);
        if (j < 0 || rhs.manifest_[j].quantity() != manifest_[i].quantity()) return false;
    }
    return true;
}

void TransportVehicle::addRaw(const ReliefCargo& cargo) {
    if (count_ == capacity_) grow();
    manifest_[count_++] = cargo;
}

bool TransportVehicle::hasDuplicateCargo() const noexcept {
    for (std::size_t i = 0; i < count_; ++i)
        for (std::size_t j = i + 1; j < count_; ++j)
            if (manifest_[i] == manifest_[j]) return true;
    return false;
}

void TransportVehicle::mergeDuplicates() {
    for (std::size_t i = 0; i < count_; ++i) {
        for (std::size_t j = i + 1; j < count_;) {
            if (manifest_[i] == manifest_[j]) {
                manifest_[i] += manifest_[j].quantity();
                removeAt(j);
            } else {
                ++j;
            }
        }
    }
}

// Format: "<id> <maxPayloadKg> <count>\n" followed by <count> cargo lines.
std::ostream& operator<<(std::ostream& os, const TransportVehicle& v) {
    const auto oldPrecision = os.precision(std::numeric_limits<double>::max_digits10);
    os << v.id_ << ' ' << v.maxPayloadKg_ << ' ' << v.count_ << '\n';
    os.precision(oldPrecision);
    for (std::size_t i = 0; i < v.count_; ++i) os << v.manifest_[i] << '\n';
    return os;
}

std::istream& operator>>(std::istream& is, TransportVehicle& v) {
    std::string id;
    double maxKg = 0.0;
    std::size_t n = 0;
    if (!(is >> id >> maxKg >> n)) return is;
    TransportVehicle tmp(id, maxKg, n ? n : 1);
    for (std::size_t i = 0; i < n; ++i) {
        ReliefCargo c;
        if (!(is >> c)) return is;
        tmp.addRaw(c);
    }
    v = std::move(tmp);
    return is;
}

}  // namespace Logistics
}  // namespace ReliefOps
