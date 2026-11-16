// Unit IV: user-defined I/O manipulators for tabular manifest printing.
#pragma once

#include <iosfwd>

#include "ReliefCargo.hpp"
#include "TransportVehicle.hpp"

namespace ReliefOps {
namespace IO {

// Parameterless manipulators (usable as: os << manifestHeader;)
std::ostream& manifestHeader(std::ostream& os);
std::ostream& manifestRule(std::ostream& os);

// Parameterised manipulator: os << Indent{4} << ...
struct Indent {
    int spaces;
};
std::ostream& operator<<(std::ostream& os, Indent i);

// Parameterised manipulator: os << Kg{1234.5}  ->  "1,234.50 kg"
struct Kg {
    double value;
};
std::ostream& operator<<(std::ostream& os, Kg k);

// Formats one cargo line as a table row: os << ManifestRow{cargo}
struct ManifestRow {
    const Inventory::ReliefCargo& cargo;
};
std::ostream& operator<<(std::ostream& os, const ManifestRow& row);

void printManifest(std::ostream& os, const Logistics::TransportVehicle& v);

}  // namespace IO
}  // namespace ReliefOps
