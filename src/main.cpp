// ERL-HUB: Post-Earthquake Multi-Agency Emergency Relief Logistics Hub
// Scripted demonstration of all four CIC-211 units. Run: ./build/erl_hub
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "BinaryLog.hpp"
#include "DispatchQueue.hpp"
#include "DistributionDepot.hpp"
#include "Exceptions.hpp"
#include "GeoUtils.hpp"
#include "IncidentCommander.hpp"
#include "IoManip.hpp"
#include "ReliefCargo.hpp"
#include "ReliefVehicle.hpp"
#include "SupplyAllocation.hpp"
#include "TransportVehicle.hpp"

using namespace ReliefOps;
using namespace ReliefOps::Inventory;   // Unit I: logical namespaces ReliefOps::Inventory
using namespace ReliefOps::Logistics;   //                         and ReliefOps::Logistics

namespace {

void banner(const std::string& title) {
    std::cout << "\n=== " << title << " " << std::string(60 - std::min<std::size_t>(title.size(), 55), '=') << "\n";
}

RouteSegment seg(std::string name, double km, double rough, bool dmg, bool water, bool air, double x, double y) {
    RouteSegment s;
    s.name = std::move(name);
    s.distanceKm = km;
    s.roughness = rough;
    s.corridorDamaged = dmg;
    s.waterCrossing = water;
    s.airspaceClosed = air;
    s.endX = x;
    s.endY = y;
    return s;
}

void unitOne(DistributionDepot& depot) {
    banner("UNIT I - namespaces, overloading, references, inline, friend");
    std::cout << "Namespaces: ReliefOps::Inventory (cargo/depot), ReliefOps::Logistics (vehicles/allocation)\n";

    ReliefCargo vol = allocateSupply(depot, "WATER-20L", 3.0);  // volume based (double)
    std::cout << "Volume-based  : " << vol.quantity() << " x " << vol.id() << " fit into 3.0 m3\n";

    ReliefCargo tri = allocateSupply(depot, "TRAUMA-KIT", Triage::Critical, 40);  // triage based
    std::cout << "Triage-based  : " << tri.quantity() << " x " << tri.id() << " for 40 CRITICAL patients\n";

    MissionParameters m{"Sector-7 Rescue", 1000.0, 4, 300};
    ReliefCargo mis = allocateSupply(depot, "RATION-KIT", m);  // mission by reference
    std::cout << "Mission-based : " << mis.quantity() << " x " << mis.id() << ", remaining budget now "
              << m.payloadBudgetKg << " kg (updated through reference)\n";

    std::vector<MissionParameters> missions{{"Field Hospital", 500, 5, 80}, {"Shelter Camp", 900, 2, 600}, m};
    MissionParameters& top = highestPriorityMission(missions);  // return by reference
    top.priority = 5;  // modifies the vector element directly
    std::cout << "Highest priority mission (returned by reference): " << top.name << ", priority bumped to "
              << missions[0].priority << " in-place\n";

    std::cout << "Inline utils  : distance(0,0 -> 30,40) = " << gridDistanceKm(0, 0, 30, 40)
              << " km, utilisation(6000/8000) = " << payloadUtilisation(6000, 8000) * 100 << " %, eta = "
              << etaHours(150, 50) << " h\n";

    IncidentCommander cmd("ALPHA-ACTUAL");  // friend class
    ReliefCargo blanket = depot.item("BLANKET");
    std::cout << "Friend class  : triage of BLANKET " << blanket.triagePriority();
    cmd.overrideTriage(blanket, 5);
    std::cout << " -> " << blanket.triagePriority() << " (override by " << cmd.callsign() << ")\n";
}

void unitTwo(DistributionDepot& depot) {
    banner("UNIT II - classes, static data, dynamic memory, operator overloading");
    ReliefCargo::resetStatistics();

    // operator += restock
    depot += ReliefCargo("RATION-KIT", Category::RationKit, 200, 2.5, 0.01, 2);
    std::cout << "After += restock RATION-KIT now " << depot.item("RATION-KIT").quantity() << " units\n";

    // dynamic constructor + copy constructor (deep copy)
    TransportVehicle truck("TRK-01", 6000.0, 2);
    truck += ReliefCargo("RATION-KIT", Category::RationKit, 400, 2.5, 0.01, 2);
    truck += ReliefCargo("WATER-20L", Category::CleanWater, 60, 20.0, 0.02, 3);
    truck += ReliefCargo("TRAUMA-KIT", Category::TraumaMedicine, 25, 4.0, 0.015, 5);  // forces grow()
    TransportVehicle copy(truck);
    std::cout << "Dynamic ctor slots=" << truck.slots() << ", deep copy made, manifests equal (==): " << std::boolalpha
              << (copy == truck) << "\n";
    copy -= ReliefCargo("WATER-20L", Category::CleanWater, 10, 20.0, 0.02, 3);
    std::cout << "After -= on the copy only, manifests equal: " << (copy == truck) << " (original untouched)\n";

    // duplicate detection via == (only possible when data arrives un-merged, e.g. deserialisation)
    std::stringstream raw("DUP-1 5000 2\nWATER-20L 2 10 20 0.02 3\nWATER-20L 2 5 20 0.02 3\n");
    TransportVehicle dup;
    raw >> dup;
    std::cout << "Parsed '" << dup.id() << "' via >> ; duplicate cargo detected: " << dup.hasDuplicateCargo();
    dup.mergeDuplicates();
    std::cout << " -> after merge: " << dup.size() << " line(s), qty " << dup.at(0).quantity() << "\n";

    // departure: depot -= updates static tonnage
    depot -= ReliefCargo("RATION-KIT", Category::RationKit, 400, 2.5, 0.01, 2);
    depot -= ReliefCargo("WATER-20L", Category::CleanWater, 60, 20.0, 0.02, 3);
    std::cout << "Static analytics: total dispatched = " << ReliefCargo::totalDispatchedTonnes() << " t\n";

    std::cout << "Serialisation (<<): " << depot.item("RATION-KIT") << "\n";
}

void unitThree() {
    banner("UNIT III - inheritance, virtual base classes, containership, polymorphism");
    ReliefVehicle::setVerboseLifecycle(true);

    std::vector<ReliefVehicle*> fleet;  // base pointers
    fleet.push_back(new GroundConvoy("GC-01"));
    fleet.push_back(new AerialTransport("AT-01"));
    fleet.push_back(new MarineVessel("MV-01"));
    fleet.push_back(new AmphibiousCraft("AM-01"));  // diamond: one shared ReliefVehicle subobject
    std::cout << "Operational vehicles (static): " << ReliefVehicle::totalOperationalVehicles() << "\n";

    const std::vector<RouteSegment> legs = {
        seg("Ridge Road", 90, 0.4, false, false, false, 20, 30),
        seg("Collapsed Bridge Corridor", 40, 0.9, true, false, false, 30, 35),
        seg("Flooded River Crossing", 25, 0.2, false, true, false, 40, 40),
        seg("Storm-Cell Airspace", 120, 0.0, false, false, true, 90, 60),
    };

    for (const RouteSegment& leg : legs) {
        std::cout << "\nLeg: " << leg.name << "\n";
        for (ReliefVehicle* v : fleet) {  // runtime polymorphism via base pointer
            try {
                const double eta = v->evaluateRoute(leg);
                std::cout << "  " << v->kind() << " " << v->id() << " -> ETA " << eta << " h\n";
            } catch (const DamagedCorridorException& e) {
                std::cout << "  " << v->kind() << " " << v->id() << " BLOCKED: " << e.what() << "\n";
            } catch (const FuelDepletionException& e) {
                std::cout << "  " << v->kind() << " " << v->id() << " FUEL: " << e.what() << "\n";
            }
        }
    }

    auto* amph = dynamic_cast<AmphibiousCraft*>(fleet.back());
    std::cout << "\nContainership: " << amph->id() << " holds GPS beacon + life support (active="
              << amph->lifeSupport().active() << ", O2 min=" << amph->lifeSupport().oxygenMinutes() << ")\n";
    amph->travel(legs[2]);
    std::cout << "After travel GPS waypoint = '" << amph->gps().lastWaypoint() << "', fuel left = " << amph->fuelL()
              << " L\n";

    std::cout << "\nDeleting through base pointers (virtual destructors run derived -> base):\n";
    for (ReliefVehicle* v : fleet) delete v;
    ReliefVehicle::setVerboseLifecycle(false);
    std::cout << "Operational vehicles now: " << ReliefVehicle::totalOperationalVehicles() << "\n";
}

void unitFour(DistributionDepot& depot) {
    banner("UNIT IV - exceptions, binary streams, templates, STL, functors, manipulators");

    // --- exceptions
    GroundConvoy convoy("GC-77");
    try {
        convoy.load(ReliefCargo("WATER-20L", Category::CleanWater, 500, 20.0, 0.02, 3));  // 10 t > 8 t
    } catch (const CargoOverweightException& e) {
        std::cout << "[caught] " << e.what() << "\n";
    }
    try {
        convoy.travel(seg("Far Valley", 2000, 0.1, false, false, false, 0, 0));
    } catch (const FuelDepletionException& e) {
        std::cout << "[caught] " << e.what() << "\n";
    }
    try {
        convoy.travel(seg("Landslide Pass", 10, 1.0, true, false, false, 0, 0));
    } catch (const std::exception& e) {  // all derive from std::exception
        std::cout << "[caught via std::exception&] " << e.what() << "\n";
    }

    // --- priority queue template with functor
    PriorityDispatchQueue<DispatchRequest, LessUrgent> queue;
    queue.push({"REQ-1", "RATION-KIT", 100, 2, 12.0});
    queue.push({"REQ-2", "TRAUMA-KIT", 20, 5, 1.0});
    queue.push({"REQ-3", "BLANKET", 300, 3, 30.0});
    queue.push({"REQ-4", "WATER-20L", 50, 4, 6.0});
    std::cout << "\nDispatch order (heap pops):";
    PriorityDispatchQueue<DispatchRequest, LessUrgent> copy = queue;
    while (!copy.empty()) std::cout << ' ' << copy.pop().requestId;
    std::cout << "\n";

    PriorityDispatchQueue<int> ints;  // same template, different T
    for (int v : {4, 9, 1}) ints.push(v);
    std::cout << "PriorityDispatchQueue<int> top = " << ints.top() << "\n";

    // --- STL: vector + functor sort, unordered_map index
    std::vector<DispatchRequest> reqs = queue.orderedSnapshot();
    std::reverse(reqs.begin(), reqs.end());
    std::sort(reqs.begin(), reqs.end(), MostUrgentFirst());
    std::unordered_map<std::string, int> demandByItem;
    for (const auto& r : reqs) demandByItem[r.itemId] += r.quantity;
    std::cout << "Sorted by MostUrgentFirst functor:";
    for (const auto& r : reqs) std::cout << ' ' << r.requestId << "(score " << r.urgencyScore() << ")";
    std::cout << "\nunordered_map demand: BLANKET=" << demandByItem["BLANKET"] << " TRAUMA-KIT=" << demandByItem["TRAUMA-KIT"]
              << "\n";

    // --- manipulators + manifest printing
    TransportVehicle van("VAN-9", 4000.0);
    for (const ReliefCargo& c : depot.snapshot()) {
        ReliefCargo part(c.id(), c.category(), std::min(c.quantity(), 50), c.unitWeightKg(), c.unitVolumeM3(),
                         c.triagePriority());
        van += part;
    }
    std::cout << "\n";
    IO::printManifest(std::cout, van);
    std::cout << IO::Indent{4} << "Total manifest weight: " << IO::Kg{van.loadedKg()} << "\n";

    // --- binary persistence with atomic replace
    const std::string manifestPath = "manifest_audit.bin";
    const std::string logPath = "transit_log.bin";
    std::vector<Persistence::TransitLogEntry> log{
        {static_cast<std::int64_t>(std::time(nullptr)), "GC-01", "DEPART Sector-7", 5200.0},
        {static_cast<std::int64_t>(std::time(nullptr)), "AT-01", "AIRLIFT Field Hospital", 900.0},
    };
    Persistence::BinaryLog::saveTransitLogs(logPath, log);
    Persistence::BinaryLog::saveManifest(manifestPath, van);
    auto restoredLog = Persistence::BinaryLog::loadTransitLogs(logPath);
    TransportVehicle restored = Persistence::BinaryLog::loadManifest(manifestPath);
    std::cout << "Saved + restored " << restoredLog.size() << " transit log(s); manifest identical after reload: "
              << std::boolalpha << (restored == van) << " (" << manifestPath << ", " << logPath << ")\n";
    try {
        Persistence::BinaryLog::loadManifest("does_not_exist.bin");
    } catch (const PersistenceException& e) {
        std::cout << "[caught] " << e.what() << "\n";
    }
    std::remove(manifestPath.c_str());
    std::remove(logPath.c_str());
}

}  // namespace

int main() {
    std::cout << "ERL-HUB :: Post-Earthquake Multi-Agency Emergency Relief Logistics Hub\n";

    DistributionDepot depot("Depot Alpha (Regional Hub)", 40.0);
    depot += ReliefCargo("RATION-KIT", Category::RationKit, 1000, 2.5, 0.010, 2);
    depot += ReliefCargo("TRAUMA-KIT", Category::TraumaMedicine, 120, 4.0, 0.015, 5);
    depot += ReliefCargo("WATER-20L", Category::CleanWater, 300, 20.0, 0.020, 3);
    depot += ReliefCargo("BLANKET", Category::ThermalBlanket, 800, 0.6, 0.004, 1);

    try {
        unitOne(depot);
        unitTwo(depot);
        unitThree();
        unitFour(depot);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    std::cout << "\nAll four units demonstrated successfully.\n";
    return 0;
}
