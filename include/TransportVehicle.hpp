// Unit II: TransportVehicle - owns a dynamically allocated cargo manifest.
// Demonstrates dynamic constructor, copy constructor (deep copy), copy
// assignment, move operations, destructor and operator overloading.
#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

#include "Forward.hpp"
#include "ReliefCargo.hpp"

namespace ReliefOps {
namespace Logistics {

class TransportVehicle {
public:
    TransportVehicle();                                                  // default
    TransportVehicle(std::string id, double maxPayloadKg, std::size_t initialSlots = 4);  // dynamic ctor
    TransportVehicle(const TransportVehicle& other);                     // copy ctor (deep copy)
    TransportVehicle(TransportVehicle&& other) noexcept;
    TransportVehicle& operator=(TransportVehicle other) noexcept;        // copy-and-swap
    ~TransportVehicle();

    const std::string& id() const noexcept { return id_; }
    double maxPayloadKg() const noexcept { return maxPayloadKg_; }
    double loadedKg() const noexcept;
    std::size_t size() const noexcept { return count_; }
    std::size_t slots() const noexcept { return capacity_; }
    const Inventory::ReliefCargo& at(std::size_t i) const;

    // Load cargo (merges identical items). Throws CargoOverweightException.
    TransportVehicle& operator+=(const Inventory::ReliefCargo& cargo);
    // Unload cargo. Throws InsufficientStockException.
    TransportVehicle& operator-=(const Inventory::ReliefCargo& cargo);
    // Manifest identity: same items with same quantities (order-insensitive).
    bool operator==(const TransportVehicle& rhs) const noexcept;
    bool operator!=(const TransportVehicle& rhs) const noexcept { return !(*this == rhs); }

    // Appends without merging/limit check (used by deserialization) - may create duplicates.
    void addRaw(const Inventory::ReliefCargo& cargo);
    bool hasDuplicateCargo() const noexcept;
    void mergeDuplicates();

    friend std::ostream& operator<<(std::ostream& os, const TransportVehicle& v);
    friend std::istream& operator>>(std::istream& is, TransportVehicle& v);

    friend class ReliefOps::IncidentCommander;

private:
    void grow();
    int indexOf(const Inventory::ReliefCargo& cargo) const noexcept;
    void removeAt(std::size_t index) noexcept;
    friend void swap(TransportVehicle& a, TransportVehicle& b) noexcept;

    std::string id_;
    double maxPayloadKg_;
    Inventory::ReliefCargo* manifest_;  // heap-allocated array
    std::size_t count_;
    std::size_t capacity_;
};

void swap(TransportVehicle& a, TransportVehicle& b) noexcept;

}  // namespace Logistics
}  // namespace ReliefOps
