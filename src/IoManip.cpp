#include "IoManip.hpp"

#include <iomanip>
#include <ostream>
#include <sstream>

namespace ReliefOps {
namespace IO {

std::ostream& manifestHeader(std::ostream& os) {
    const auto flags = os.flags();
    os << std::left << std::setw(14) << "ITEM-ID" << std::setw(18) << "CATEGORY" << std::right
       << std::setw(7) << "QTY" << std::setw(12) << "WEIGHT(kg)" << std::setw(8) << "TRIAGE" << '\n';
    os.flags(flags);
    return os;
}

std::ostream& manifestRule(std::ostream& os) {
    return os << std::string(59, '-') << '\n';
}

std::ostream& operator<<(std::ostream& os, Indent i) {
    return os << std::string(static_cast<std::size_t>(i.spaces > 0 ? i.spaces : 0), ' ');
}

std::ostream& operator<<(std::ostream& os, Kg k) {
    // group thousands manually so output is locale independent
    std::ostringstream tmp;
    tmp << std::fixed << std::setprecision(2) << k.value;
    std::string s = tmp.str();
    const std::size_t dot = s.find('.');
    const std::size_t start = (s[0] == '-') ? 1 : 0;
    for (std::size_t pos = dot; pos > start + 3;) {
        pos -= 3;
        s.insert(pos, ",");
    }
    return os << s << " kg";
}

std::ostream& operator<<(std::ostream& os, const ManifestRow& row) {
    const auto flags = os.flags();
    const auto& c = row.cargo;
    os << std::left << std::setw(14) << c.id() << std::setw(18) << Inventory::toString(c.category())
       << std::right << std::setw(7) << c.quantity() << std::setw(12) << std::fixed << std::setprecision(1)
       << c.totalWeightKg() << std::setw(8) << c.triagePriority();
    os.flags(flags);
    return os;
}

void printManifest(std::ostream& os, const Logistics::TransportVehicle& v) {
    os << "Manifest for " << v.id() << "  (load " << Kg{v.loadedKg()} << " / " << Kg{v.maxPayloadKg()}
       << ")\n"
       << manifestRule << manifestHeader << manifestRule;
    for (std::size_t i = 0; i < v.size(); ++i) os << ManifestRow{v.at(i)} << '\n';
    os << manifestRule;
}

}  // namespace IO
}  // namespace ReliefOps
