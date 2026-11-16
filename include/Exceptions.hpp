// Unit IV: custom exception hierarchy rooted at std::exception.
#pragma once

#include <exception>
#include <string>
#include <utility>

namespace ReliefOps {

class ErlException : public std::exception {
public:
    explicit ErlException(std::string message) : message_(std::move(message)) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

class CargoOverweightException : public ErlException {
public:
    explicit CargoOverweightException(double excessKg)
        : ErlException("Cargo overweight: payload limit exceeded by " + std::to_string(excessKg) + " kg"),
          excessKg_(excessKg) {}
    double excessKg() const noexcept { return excessKg_; }

private:
    double excessKg_;
};

class FuelDepletionException : public ErlException {
public:
    FuelDepletionException(double neededL, double availableL)
        : ErlException("Fuel depletion: need " + std::to_string(neededL) + " L, only " +
                       std::to_string(availableL) + " L available") {}
};

class DamagedCorridorException : public ErlException {
public:
    explicit DamagedCorridorException(const std::string& segment)
        : ErlException("Damaged/impassable corridor: " + segment) {}
};

class InsufficientStockException : public ErlException {
public:
    explicit InsufficientStockException(const std::string& detail)
        : ErlException("Insufficient stock: " + detail) {}
};

class DepotCapacityException : public ErlException {
public:
    explicit DepotCapacityException(const std::string& depot)
        : ErlException("Depot capacity exceeded at " + depot) {}
};

class PersistenceException : public ErlException {
public:
    explicit PersistenceException(const std::string& detail)
        : ErlException("Persistence error: " + detail) {}
};

}  // namespace ReliefOps
