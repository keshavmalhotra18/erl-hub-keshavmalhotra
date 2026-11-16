// Unit III: independent components used via containership (composition)
// inside the vehicle classes instead of deep inheritance.
#pragma once

#include <string>
#include <utility>

namespace ReliefOps {
namespace Logistics {

class GPSBeacon {
public:
    GPSBeacon() : x_(0.0), y_(0.0), lastWaypoint_("BASE"), pings_(0) {}
    void update(double x, double y, std::string waypoint) {
        x_ = x;
        y_ = y;
        lastWaypoint_ = std::move(waypoint);
        ++pings_;
    }
    double x() const noexcept { return x_; }
    double y() const noexcept { return y_; }
    const std::string& lastWaypoint() const noexcept { return lastWaypoint_; }
    int pings() const noexcept { return pings_; }

private:
    double x_, y_;
    std::string lastWaypoint_;
    int pings_;
};

class EmergencyLifeSupportUnit {
public:
    explicit EmergencyLifeSupportUnit(int oxygenMinutes = 240, int medKits = 2)
        : oxygenMinutes_(oxygenMinutes), medKits_(medKits), active_(false) {}
    void activate() noexcept { active_ = true; }
    void deactivate() noexcept { active_ = false; }
    bool active() const noexcept { return active_; }
    int oxygenMinutes() const noexcept { return oxygenMinutes_; }
    int medKits() const noexcept { return medKits_; }
    bool useMedKit() noexcept {
        if (medKits_ <= 0) return false;
        --medKits_;
        return true;
    }
    void consumeOxygen(int minutes) noexcept {
        if (!active_) return;
        oxygenMinutes_ = oxygenMinutes_ > minutes ? oxygenMinutes_ - minutes : 0;
    }

private:
    int oxygenMinutes_;
    int medKits_;
    bool active_;
};

}  // namespace Logistics
}  // namespace ReliefOps
