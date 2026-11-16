// Unit IV: binary persistence with std::fstream and atomic file replacement.
//
// File layout (little-endian host order):
//   "ERLH" | u32 version | u32 kind | payload... | u32 FNV-1a checksum of everything before it
// Atomicity: data is written to "<path>.tmp", flushed, verified, then renamed over <path>.
// A crash mid-write therefore never corrupts the previous good file.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "TransportVehicle.hpp"

namespace ReliefOps {
namespace Persistence {

struct TransitLogEntry {
    std::int64_t timestamp = 0;
    std::string vehicleId;
    std::string event;
    double cargoKg = 0.0;
};

bool operator==(const TransitLogEntry& a, const TransitLogEntry& b) noexcept;

class BinaryLog {
public:
    static void saveTransitLogs(const std::string& path, const std::vector<TransitLogEntry>& entries);
    static std::vector<TransitLogEntry> loadTransitLogs(const std::string& path);

    static void saveManifest(const std::string& path, const Logistics::TransportVehicle& vehicle);
    static Logistics::TransportVehicle loadManifest(const std::string& path);
};

}  // namespace Persistence
}  // namespace ReliefOps
