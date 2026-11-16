// Unit I: inline utility functions for rapid distance / payload calculations.
#pragma once

#include <cmath>

namespace ReliefOps {
namespace Logistics {

// Straight-line distance between two grid points (km).
inline double gridDistanceKm(double x1, double y1, double x2, double y2) noexcept {
    return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Fraction of the payload limit currently used (0..1+).
inline double payloadUtilisation(double loadedKg, double maxKg) noexcept {
    return maxKg > 0.0 ? loadedKg / maxKg : 0.0;
}

// Estimated travel time in hours.
inline double etaHours(double distanceKm, double speedKmh) noexcept {
    return speedKmh > 0.0 ? distanceKm / speedKmh : 0.0;
}

}  // namespace Logistics
}  // namespace ReliefOps
