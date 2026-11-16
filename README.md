# ERL-HUB — Post-Earthquake Multi-Agency Emergency Relief Logistics Hub

CIC-211 Object-Oriented Programming Using C++ project (Option 2). Standard C++17, no external dependencies.

**Author:** Keshav Malhotra · Roll No. 35014802725 · Class Group C-18

## Build & run

```bash
make          # builds build/erl_hub and build/erl_tests
make run      # scripted demo of all four syllabus units
make test     # 38 dependency-free unit checks
make clean
```
Requires a C++17 compiler (g++ ≥ 8 / clang++ ≥ 7) and `make`. Compiled with `-Wall -Wextra -Wpedantic` and zero warnings.

## Scenario

After a major earthquake, a regional field hub stocks relief supplies (ration kits, trauma medicine, clean water,
thermal blankets) and dispatches heterogeneous vehicles (all-terrain convoys, helicopters/quadcopters, boats and
amphibious medical units) over damaged infrastructure. The system allocates scarce stock by volume, mission budget or
medical triage, validates routes per vehicle type, and persists audit records safely.

## Project layout

| Path | Purpose |
|---|---|
| `include/Exceptions.hpp` | Exception hierarchy (all derive from `std::exception`) |
| `include/GeoUtils.hpp` | `inline` distance / payload / ETA utilities |
| `include/ReliefCargo.hpp`, `src/ReliefCargo.cpp` | `ReliefOps::Inventory::ReliefCargo` |
| `include/DistributionDepot.hpp`, `src/…` | `ReliefOps::Inventory::DistributionDepot` |
| `include/TransportVehicle.hpp`, `src/…` | `ReliefOps::Logistics::TransportVehicle` (dynamic manifest) |
| `include/Components.hpp` | `GPSBeacon`, `EmergencyLifeSupportUnit` (composed parts) |
| `include/ReliefVehicle.hpp`, `src/…` | Vehicle hierarchy |
| `include/SupplyAllocation.hpp`, `src/…` | Overloaded allocation, reference semantics |
| `include/IncidentCommander.hpp`, `src/…` | `friend` class for triage overrides |
| `include/DispatchQueue.hpp` | `PriorityDispatchQueue<T,Compare>` template + functors |
| `include/IoManip.hpp`, `src/…` | Custom I/O manipulators, manifest table |
| `include/BinaryLog.hpp`, `src/…` | Binary `std::fstream` persistence with atomic replace |
| `src/main.cpp` | Demonstration driver (one function per unit) |
| `tests/test_main.cpp` | Unit tests |

## Syllabus requirement → implementation map

### Unit I — Core foundations & call semantics
| Requirement | Where |
|---|---|
| Namespaces `ReliefOps::Logistics`, `ReliefOps::Inventory` | `ReliefCargo`/`DistributionDepot` in `Inventory`; vehicles, allocation, queue in `Logistics` |
| Function overloading (volume vs. triage) | Three `allocateSupply(...)` overloads in `SupplyAllocation.hpp`: `(depot,id,double volumeM3)`, `(depot,id,Triage,int patients)`, `(depot,id,MissionParameters&)` |
| Call by reference | `allocateSupply(..., MissionParameters&)` updates the mission budget in place; `dispatchToVehicle(depot&, vehicle&, …)` |
| Return by reference | `DistributionDepot::item()` and `highestPriorityMission()` return references that callers modify in place |
| Inline utilities | `gridDistanceKm`, `payloadUtilisation`, `etaHours` (`GeoUtils.hpp`) |
| Friend class `IncidentCommander` | Overrides private triage level, fuel, payload limit and depot capacity |

### Unit II — Encapsulation, dynamic memory & operator overloading
| Requirement | Where |
|---|---|
| Classes `ReliefCargo`, `DistributionDepot`, `TransportVehicle` with data hiding | Private state, const-correct accessors |
| Static variables / functions | `ReliefCargo::totalDispatchedTonnes()/recordDispatch()`, `ReliefVehicle::totalOperationalVehicles()` |
| Dynamic allocation: dynamic ctor, copy ctor, destructor | `TransportVehicle` owns a `new[]` manifest: parameterised dynamic constructor, deep-copy ctor, copy-and-swap assignment, move ctor, `delete[]` destructor, growth on demand |
| `+=` restock | `DistributionDepot::operator+=`, `ReliefCargo::operator+=`, `TransportVehicle::operator+=` (load) |
| `-=` deduct on departure | `DistributionDepot::operator-=` (updates dispatched tonnage), `ReliefCargo`/`TransportVehicle` `-=` |
| `==` manifest identity / duplicates | `ReliefCargo::operator==` (id + category), `TransportVehicle::operator==` (order-insensitive manifest equality), `hasDuplicateCargo()` |
| `<<` and `>>` | Text serialisation round-trip for `ReliefCargo` and `TransportVehicle` |

### Unit III — Inheritance, containership & polymorphism
| Requirement | Where |
|---|---|
| Base `ReliefVehicle` → `AerialTransport`, `GroundConvoy`, `AmphibiousCraft` | `ReliefVehicle.hpp` (abstract, copy disabled) |
| Virtual base classes for the diamond | `GroundConvoy` and `MarineVessel` inherit `virtual ReliefVehicle`; `AmphibiousCraft : GroundConvoy, MarineVessel` gets exactly **one** `ReliefVehicle` subobject and initialises it directly |
| Containership | `ReliefVehicle` *has-a* `GPSBeacon`, `EmergencyLifeSupportUnit` and `TransportVehicle` hold (no deep inheritance) |
| Runtime polymorphism via `ReliefVehicle*`, virtual destructors | `main.cpp` stores `std::vector<ReliefVehicle*>`, calls `evaluateRoute()`, and deletes via base pointers; verbose lifecycle output shows the correct destructor chain |

Diamond:
```
            ReliefVehicle (abstract)
        /-------/   |    \-------\   (virtual)
 AerialTransport GroundConvoy MarineVessel
                      \         /
                   AmphibiousCraft
```

### Unit IV — Streams, persistence, exceptions & STL
| Requirement | Where |
|---|---|
| Exceptions deriving from `std::exception` | `CargoOverweightException`, `FuelDepletionException`, `DamagedCorridorException` (+ `InsufficientStock`, `DepotCapacity`, `Persistence`) via `ErlException` |
| Binary streams with `std::fstream`, atomic updates | `BinaryLog`: magic + version + kind header, FNV-1a checksum; writes `<file>.tmp`, flushes, then `std::filesystem::rename` over the target. Corrupt/truncated files raise `PersistenceException` |
| Template `PriorityDispatchQueue<T>` | `DispatchQueue.hpp` — heap on `std::vector`; instantiated for `DispatchRequest` and `int` |
| STL containers | `std::vector` (queue storage, manifests/logs), `std::unordered_map` (depot stock, demand index) |
| Functors | `LessUrgent` (heap ordering), `MostUrgentFirst` (`std::sort`) |
| User-defined I/O manipulators | `manifestHeader`, `manifestRule`, `Indent{n}`, `Kg{x}`, `ManifestRow{c}`, `printManifest()` |

## Design notes
* **Exception safety:** `TransportVehicle::operator+=` and `dispatchToVehicle()` validate before mutating, so a
  `CargoOverweightException` leaves both vehicle and depot unchanged. Assignment uses copy-and-swap.
* **Serialisation format:** `ReliefCargo` text lines are `id category qty unitKg unitM3 triage` (ids contain no spaces).
* **Triage scale:** 1 (routine) – 5 (critical); urgency score = `triage × 10 + hoursWaiting`.
* **Amphibious behaviour:** water legs use marine rules, land legs ground rules; it cannot cross a damaged, non-flooded corridor.
