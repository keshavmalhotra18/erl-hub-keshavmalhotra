// Dependency-free unit tests. Run: make test
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#include "BinaryLog.hpp"
#include "DispatchQueue.hpp"
#include "DistributionDepot.hpp"
#include "Exceptions.hpp"
#include "IncidentCommander.hpp"
#include "ReliefVehicle.hpp"
#include "SupplyAllocation.hpp"
#include "TransportVehicle.hpp"

using namespace ReliefOps;
using namespace ReliefOps::Inventory;
using namespace ReliefOps::Logistics;

static int g_failed = 0, g_total = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++g_total;                                                                    \
        if (!(cond)) {                                                                \
            ++g_failed;                                                               \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n";   \
        }                                                                             \
    } while (0)

#define CHECK_THROWS(expr, ExType)                    \
    do {                                              \
        ++g_total;                                    \
        bool caught = false;                          \
        try { expr; } catch (const ExType&) { caught = true; } catch (...) {} \
        if (!caught) { ++g_failed; std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  expected " #ExType "\n"; } \
    } while (0)

static ReliefCargo water(int q) { return ReliefCargo("WATER", Category::CleanWater, q, 20.0, 0.02, 3); }

static void testCargo() {
    ReliefCargo c = water(10);
    c += 5;
    CHECK(c.quantity() == 15);
    c -= 15;
    CHECK(c.quantity() == 0);
    CHECK_THROWS(c -= 1, InsufficientStockException);
    CHECK(water(1) == water(99));
    CHECK(!(water(1) == ReliefCargo("OTHER", Category::CleanWater, 1, 1, 1)));
    std::stringstream ss;
    ss << water(7);
    ReliefCargo back;
    ss >> back;
    CHECK(back.quantity() == 7 && back.id() == "WATER" && back.unitWeightKg() == 20.0);
}

static void testDepot() {
    ReliefCargo::resetStatistics();
    DistributionDepot d("D", 1.0);
    d += water(10);
    d += water(5);
    CHECK(d.item("WATER").quantity() == 15);
    CHECK_THROWS(d += water(1000), DepotCapacityException);
    d -= water(15);
    CHECK(!d.has("WATER"));
    CHECK(ReliefCargo::totalDispatchedTonnes() > 0.29 && ReliefCargo::totalDispatchedTonnes() < 0.31);
    CHECK_THROWS(d -= water(1), InsufficientStockException);
}

static void testTransportVehicle() {
    TransportVehicle t("T", 500.0, 1);
    t += water(10);  // 200 kg
    t += ReliefCargo("B", Category::ThermalBlanket, 10, 1.0, 0.01);
    t += water(5);  // merged
    CHECK(t.size() == 2);
    CHECK_THROWS(t += water(100), CargoOverweightException);
    TransportVehicle copy(t);
    CHECK(copy == t);
    copy -= water(1);
    CHECK(copy != t);
    CHECK(t.at(0).quantity() == 15);  // deep copy: original unchanged
    TransportVehicle assigned;
    assigned = t;
    CHECK(assigned == t);
    std::stringstream ss;
    ss << t;
    TransportVehicle parsed;
    ss >> parsed;
    CHECK(parsed == t);
}

static void testPolymorphism() {
    const int before = ReliefVehicle::totalOperationalVehicles();
    ReliefVehicle* v = new AmphibiousCraft("AM");
    CHECK(ReliefVehicle::totalOperationalVehicles() == before + 1);  // single virtual base
    CHECK(v->kind() == "AmphibiousCraft");
    RouteSegment water_leg;
    water_leg.name = "river";
    water_leg.distanceKm = 28;
    water_leg.waterCrossing = true;
    CHECK(v->evaluateRoute(water_leg) > 0.0);
    RouteSegment broken;
    broken.name = "bridge";
    broken.distanceKm = 5;
    broken.corridorDamaged = true;
    CHECK_THROWS(v->evaluateRoute(broken), DamagedCorridorException);
    delete v;  // virtual destructor via base pointer
    CHECK(ReliefVehicle::totalOperationalVehicles() == before);

    GroundConvoy g("G");
    RouteSegment far;
    far.name = "far";
    far.distanceKm = 5000;
    CHECK_THROWS(g.travel(far), FuelDepletionException);
    IncidentCommander cmd("X");
    cmd.emergencyRefuel(g, 5000.0);
    CHECK(g.fuelL() > 5000.0);
    cmd.overridePayloadLimit(g, 100000.0);
    g.load(water(1000));  // would exceed the stock 8 t limit otherwise
}

static void testAllocationAndQueue() {
    DistributionDepot d("D", 100.0);
    d += water(100);
    CHECK(allocateSupply(d, "WATER", 0.1).quantity() == 5);                    // volume based
    CHECK(allocateSupply(d, "WATER", Triage::Critical, 10).quantity() == 30);  // triage based
    MissionParameters m{"m", 100.0, 1, 0};
    CHECK(allocateSupply(d, "WATER", m).quantity() == 5);
    CHECK(m.payloadBudgetKg == 0.0);
    std::vector<MissionParameters> ms{{"a", 0, 1, 0}, {"b", 0, 9, 0}};
    highestPriorityMission(ms).priority = 1;
    CHECK(ms[1].priority == 1);

    PriorityDispatchQueue<DispatchRequest, LessUrgent> q;
    q.push({"low", "x", 1, 1, 0});
    q.push({"high", "x", 1, 5, 0});
    q.push({"mid", "x", 1, 3, 0});
    CHECK(q.pop().requestId == "high");
    CHECK(q.pop().requestId == "mid");
    CHECK(q.pop().requestId == "low");
    CHECK_THROWS(q.pop(), ErlException);
}

static void testPersistence() {
    const std::string p = "test_manifest.bin", l = "test_log.bin";
    TransportVehicle t("T", 1000.0);
    t += water(10);
    Persistence::BinaryLog::saveManifest(p, t);
    CHECK(Persistence::BinaryLog::loadManifest(p) == t);
    std::vector<Persistence::TransitLogEntry> log{{123, "V", "EVT", 1.5}};
    Persistence::BinaryLog::saveTransitLogs(l, log);
    CHECK(Persistence::BinaryLog::loadTransitLogs(l) == log);
    // corrupt a byte -> checksum must fail
    {
        std::fstream f(p, std::ios::in | std::ios::out | std::ios::binary);
        f.seekp(12);
        f.put('\x7f');
    }
    CHECK_THROWS(Persistence::BinaryLog::loadManifest(p), PersistenceException);
    CHECK_THROWS(Persistence::BinaryLog::loadManifest("missing.bin"), PersistenceException);
    std::remove(p.c_str());
    std::remove(l.c_str());
}

int main() {
    testCargo();
    testDepot();
    testTransportVehicle();
    testPolymorphism();
    testAllocationAndQueue();
    testPersistence();
    std::cout << (g_total - g_failed) << "/" << g_total << " checks passed\n";
    return g_failed == 0 ? 0 : 1;
}
