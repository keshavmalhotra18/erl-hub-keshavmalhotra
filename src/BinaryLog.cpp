#include "BinaryLog.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <cstdio>

#include "Exceptions.hpp"

namespace ReliefOps {
namespace Persistence {

namespace {

constexpr char kMagic[4] = {'E', 'R', 'L', 'H'};
constexpr std::uint32_t kVersion = 1;
constexpr std::uint32_t kKindTransit = 1;
constexpr std::uint32_t kKindManifest = 2;

std::uint32_t fnv1a(const std::string& data) noexcept {
    std::uint32_t h = 2166136261u;
    for (unsigned char c : data) {
        h ^= c;
        h *= 16777619u;
    }
    return h;
}

class Writer {
public:
    template <typename T>
    void put(T value) {
        buf_.append(reinterpret_cast<const char*>(&value), sizeof(T));
    }
    void putString(const std::string& s) {
        put<std::uint32_t>(static_cast<std::uint32_t>(s.size()));
        buf_.append(s);
    }
    void header(std::uint32_t kind) {
        buf_.append(kMagic, 4);
        put<std::uint32_t>(kVersion);
        put<std::uint32_t>(kind);
    }
    std::string finish() {
        put<std::uint32_t>(fnv1a(buf_));
        return std::move(buf_);
    }

private:
    std::string buf_;
};

class Reader {
public:
    Reader(const std::string& data, std::size_t end) : data_(data), pos_(0), end_(end) {}
    template <typename T>
    T get() {
        need(sizeof(T));
        T v;
        std::memcpy(&v, data_.data() + pos_, sizeof(T));
        pos_ += sizeof(T);
        return v;
    }
    std::string getString() {
        const auto len = get<std::uint32_t>();
        need(len);
        std::string s = data_.substr(pos_, len);
        pos_ += len;
        return s;
    }
    void expectHeader(std::uint32_t kind) {
        need(4);
        if (std::memcmp(data_.data(), kMagic, 4) != 0) throw PersistenceException("bad magic number");
        pos_ += 4;
        if (get<std::uint32_t>() != kVersion) throw PersistenceException("unsupported version");
        if (get<std::uint32_t>() != kind) throw PersistenceException("unexpected file kind");
    }

private:
    void need(std::size_t n) const {
        if (n > end_ - pos_) throw PersistenceException("truncated or corrupt file");
    }
    const std::string& data_;
    std::size_t pos_;
    std::size_t end_;
};

void atomicWrite(const std::string& path, const std::string& bytes) {
    const std::string tmp = path + ".tmp";
    {
        std::fstream out(tmp, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out) throw PersistenceException("cannot open " + tmp + " for writing");
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        out.flush();
        if (!out) {
            out.close();
            std::error_code ignore;
            std::filesystem::remove(tmp, ignore);
            throw PersistenceException("write failed for " + tmp);
        }
    }  // closed here
    std::error_code ec;
    std::filesystem::rename(tmp, path, ec);  // atomic replace on POSIX; replaces on Windows too
    if (ec) {
        std::filesystem::remove(tmp, ec);
        throw PersistenceException("cannot replace " + path);
    }
}

std::string readVerified(const std::string& path, std::size_t& payloadEnd) {
    std::fstream in(path, std::ios::in | std::ios::binary);
    if (!in) throw PersistenceException("cannot open " + path);
    std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (data.size() < 16) throw PersistenceException("file too small: " + path);
    payloadEnd = data.size() - sizeof(std::uint32_t);
    std::uint32_t stored;
    std::memcpy(&stored, data.data() + payloadEnd, sizeof(stored));
    if (stored != fnv1a(data.substr(0, payloadEnd))) throw PersistenceException("checksum mismatch: " + path);
    return data;
}

}  // namespace

bool operator==(const TransitLogEntry& a, const TransitLogEntry& b) noexcept {
    return a.timestamp == b.timestamp && a.vehicleId == b.vehicleId && a.event == b.event &&
           a.cargoKg == b.cargoKg;
}

void BinaryLog::saveTransitLogs(const std::string& path, const std::vector<TransitLogEntry>& entries) {
    Writer w;
    w.header(kKindTransit);
    w.put<std::uint64_t>(entries.size());
    for (const auto& e : entries) {
        w.put<std::int64_t>(e.timestamp);
        w.putString(e.vehicleId);
        w.putString(e.event);
        w.put<double>(e.cargoKg);
    }
    atomicWrite(path, w.finish());
}

std::vector<TransitLogEntry> BinaryLog::loadTransitLogs(const std::string& path) {
    std::size_t end = 0;
    const std::string data = readVerified(path, end);
    Reader r(data, end);
    r.expectHeader(kKindTransit);
    const auto n = r.get<std::uint64_t>();
    std::vector<TransitLogEntry> out;
    for (std::uint64_t i = 0; i < n; ++i) {
        TransitLogEntry e;
        e.timestamp = r.get<std::int64_t>();
        e.vehicleId = r.getString();
        e.event = r.getString();
        e.cargoKg = r.get<double>();
        out.push_back(std::move(e));
    }
    return out;
}

void BinaryLog::saveManifest(const std::string& path, const Logistics::TransportVehicle& v) {
    Writer w;
    w.header(kKindManifest);
    w.putString(v.id());
    w.put<double>(v.maxPayloadKg());
    w.put<std::uint64_t>(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        const auto& c = v.at(i);
        w.putString(c.id());
        w.put<std::int32_t>(static_cast<std::int32_t>(c.category()));
        w.put<std::int32_t>(c.quantity());
        w.put<double>(c.unitWeightKg());
        w.put<double>(c.unitVolumeM3());
        w.put<std::int32_t>(c.triagePriority());
    }
    atomicWrite(path, w.finish());
}

Logistics::TransportVehicle BinaryLog::loadManifest(const std::string& path) {
    std::size_t end = 0;
    const std::string data = readVerified(path, end);
    Reader r(data, end);
    r.expectHeader(kKindManifest);
    std::string id = r.getString();
    const double maxKg = r.get<double>();
    const auto n = r.get<std::uint64_t>();
    Logistics::TransportVehicle v(id, maxKg, n ? static_cast<std::size_t>(n) : 1);
    for (std::uint64_t i = 0; i < n; ++i) {
        std::string cid = r.getString();
        const auto cat = r.get<std::int32_t>();
        const auto qty = r.get<std::int32_t>();
        const double w = r.get<double>();
        const double vol = r.get<double>();
        const auto tri = r.get<std::int32_t>();
        if (cat < 0 || cat > 3 || qty < 0) throw PersistenceException("invalid cargo record");
        v.addRaw(Inventory::ReliefCargo(std::move(cid), static_cast<Inventory::Category>(cat), qty, w, vol, tri));
    }
    return v;
}

}  // namespace Persistence
}  // namespace ReliefOps
