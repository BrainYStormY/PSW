// =====================================================================
//  test_residents.cpp  -  Console test of the Resident agent (Sprint 2)
// ---------------------------------------------------------------------
//  Covers US26 - US30 (population) and US31 - US34 (behaviour).
//  NOT connected to Simulation yet: SimulationEngine.hpp does not compile
//  (Statistics::update is private), so this file does not include it.
//  The tick loop in runScenario() does what Simulation::step() will do.
//
//  Build:  g++ -std=c++17 -Wall test_residents.cpp Resident.cpp Population.cpp -o test_residents
//  Run:    ./test_residents      (exit code 0 = all checks passed)
// =====================================================================

#include "Types.hpp"
#include "Environment.hpp"
#include "Agents.hpp"
#include "Population.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>


namespace{

int failures = 0;      // number of failed checks, becomes the exit code


// ---------------------------------------------------------------------
//  Prints one check as [PASS] / [FAIL] and counts the failures
// ---------------------------------------------------------------------
void check(bool ok, const std::string& what){
    std::cout << (ok ? "  [PASS] " : "  [FAIL] ") << what << "\n";
    if(!ok) failures++;
}


// ---------------------------------------------------------------------
//  A small world that owns everything it contains (the pointers are raw,
//  like in the rest of the project, so somebody has to delete them)
// ---------------------------------------------------------------------
struct World{
    std::vector<House*> houses;
    std::vector<Bin*> bins;
    std::vector<Resident*> residents;

    World() = default;
    World(const World&) = delete;                // raw pointers: no copies
    World& operator=(const World&) = delete;
    ~World(){
        for(Resident* r : residents) delete r;
        for(House* h : houses) delete h;
        for(Bin* b : bins) delete b;
    }
};


// ---------------------------------------------------------------------
//  Example settings. They mirror the profile table of the GUI mockup
//  (python/.../gui/app.py: RESIDENT_PROFILES). In the real program these
//  values come from the GUI, never from the code.
// ---------------------------------------------------------------------
PopulationConfig makeConfig(){
    PopulationConfig cfg;
    cfg.totalResidents = 200;                                   // US26
    cfg.types = {                                               // US28, US29, US30, US34
        {"Eco-Conscious", 30, 0.9, 0.85},
        {"Standard",      50, 1.3, 0.55},
        {"Careless",      20, 1.8, 0.20},
    };
    cfg.params = {1440, 4, 1.0};    // 1440 ticks/day (1 min per tick), walk 4 cells, 1 trip/day
    return cfg;
}


// ---------------------------------------------------------------------
//  Scenario A: builds a small city, creates the population and runs it
//
//   y=2   H(2,2)  b(5,2)  H(8,2)  b(11,2)  H(14,2)
//   y=10  H(2,10) b(5,10) H(8,10) b(11,10) H(14,10)      H(28,28) = isolated house
//   plus one RECYCLING bin at (3,3), next to the first house (must be ignored)
//
//  With walkingRange = 4 the outer houses reach 1 bin, the middle houses
//  reach 2 bins and the isolated house reaches none (US31).
//  The bins are huge, so none of them is ever full here.
// ---------------------------------------------------------------------
void runScenario(World& world, const PopulationConfig& cfg, unsigned seed, int days){
    std::mt19937 rng(seed);         // ONE seeded engine for everything (US42)

    // houses
    const Position housePositions[] = {{2,2}, {8,2}, {14,2}, {2,10}, {8,10}, {14,10}, {28,28}};
    int id = 1;
    for(const Position& p : housePositions) world.houses.push_back(new House(id++, p));

    // general bins
    const Position binPositions[] = {{5,2}, {11,2}, {5,10}, {11,10}};
    id = 1;
    for(const Position& p : binPositions) world.bins.push_back(new Bin(id++, p, BinType::General, 1e9));

    // one recycling bin
    world.bins.push_back(new Bin(id++, Position{3,3}, BinType::Recycling, 1e9));

    // population (US26 - US30)
    world.residents = buildPopulation(cfg, world.houses, rng);

    // tick loop, same order as Simulation::step()
    const int totalTicks = days * cfg.params.ticksPerDay;
    for(int tick = 0; tick < totalTicks; tick++){
        for(Resident* res : world.residents) res->step(world.bins, rng);
    }
}


// ---------------------------------------------------------------------
//  Values that must be identical when the same seed is used twice (US42)
// ---------------------------------------------------------------------
std::vector<double> fingerprint(const World& world){
    std::vector<double> values;
    for(const Bin* bin : world.bins) values.push_back(bin->getFillLevel());
    double carried = 0.0;
    int trips = 0;
    for(const Resident* res : world.residents){
        carried += res->getCarriedWaste();
        trips += res->getTrips();
    }
    values.push_back(carried);
    values.push_back(trips);
    return values;
}


// ---------------------------------------------------------------------
//  US26 - US29: population size, types, shares, houses
// ---------------------------------------------------------------------
void testPopulation(const World& world, const PopulationConfig& cfg){
    std::cout << "\nUS26 - US29: population\n";

    // US26: total number of residents
    check(static_cast<int>(world.residents.size()) == cfg.totalResidents,
          "US26: " + std::to_string(cfg.totalResidents) + " residents were created");

    // US28 + US29: number of residents of each type = share of the total
    std::map<std::string, int> perType;
    for(const Resident* res : world.residents) perType[res->getTypeName()]++;
    bool sharesOk = true;
    for(const ResidentType& type : cfg.types){
        int expected = static_cast<int>(std::lround(cfg.totalResidents * type.sharePercent / 100.0));
        std::cout << "         " << std::left << std::setw(15) << type.name
                  << perType[type.name] << " residents (expected " << expected << ")\n";
        if(perType[type.name] != expected) sharesOk = false;
    }
    check(sharesOk, "US28/US29: every type has the requested percentage of residents");

    // rounding: 10 residents with 33.3 / 33.3 / 33.4 %
    PopulationConfig odd = cfg;
    odd.totalResidents = 10;
    odd.types[0].sharePercent = 33.3;
    odd.types[1].sharePercent = 33.3;
    odd.types[2].sharePercent = 33.4;
    std::vector<int> oddCounts = countResidentsPerType(odd);
    check(oddCounts[0] + oddCounts[1] + oddCounts[2] == 10,
          "US29: odd shares still add up to the total (no resident lost to rounding)");

    // US27: every resident lives in a house, at that house's position
    bool housesOk = true;
    for(const Resident* res : world.residents){
        House* house = res->getHouse();
        if(house == nullptr) housesOk = false;
        else if(house->getPosition().x != res->getPosition().x ||
                house->getPosition().y != res->getPosition().y) housesOk = false;
    }
    check(housesOk, "US27: every resident has a house and starts at its position");

    // US27: the houses know their residents, spread evenly
    std::size_t inHouses = 0, fewest = world.residents.size(), most = 0;
    for(const House* house : world.houses){
        std::size_t n = house->getResidents().size();
        inHouses += n;
        if(n < fewest) fewest = n;
        if(n > most) most = n;
    }
    check(inHouses == world.residents.size(), "US27: every resident is registered in exactly one house");
    check(most - fewest <= 1, "US27: residents are spread evenly over the houses");

    // invalid settings must be rejected with an exception
    PopulationConfig bad = cfg;
    bad.types[0].sharePercent = 40;                // now 110 %
    bool shareRejected = false;
    try{ validatePopulationConfig(bad); } catch(const std::invalid_argument&){ shareRejected = true; }
    check(shareRejected, "US29: shares that do not add up to 100 % are rejected");

    bad = cfg;
    bad.types[1].awareness = 1.5;
    bool awarenessRejected = false;
    try{ validatePopulationConfig(bad); } catch(const std::invalid_argument&){ awarenessRejected = true; }
    check(awarenessRejected, "US34: awareness outside 0..1 is rejected");
}


// ---------------------------------------------------------------------
//  US30: waste production
// ---------------------------------------------------------------------
void testWaste(const World& world, int days){
    std::cout << "\nUS30: waste production (" << days << " days)\n";

    // every kg produced is either in a bin or still at home
    bool amountOk = true;
    double disposed = 0.0;
    for(const Resident* res : world.residents){
        double produced = res->getTotalDisposed() + res->getCarriedWaste();
        if(std::fabs(produced - res->getWastePerDay() * days) > 1e-6) amountOk = false;
        disposed += res->getTotalDisposed();
    }
    check(amountOk, "US30: each resident produced exactly wastePerDay x days");

    double inBins = 0.0;
    for(const Bin* bin : world.bins) inBins += bin->getFillLevel();
    check(std::fabs(inBins - disposed) < 1e-6, "US30: all disposed waste is found in the bins (nothing lost)");
}


// ---------------------------------------------------------------------
//  US31: bins within reach
// ---------------------------------------------------------------------
void testReach(const World& world, const PopulationConfig& cfg){
    std::cout << "\nUS31: bins within reach (walking range " << cfg.params.walkingRange << " cells)\n";

    // everything a resident perceives is a General bin inside its range
    bool reachOk = true;
    for(const Resident* res : world.residents){
        for(const Bin* bin : res->reachableBins(world.bins)){
            int dist = std::abs(bin->getPosition().x - res->getPosition().x)
                     + std::abs(bin->getPosition().y - res->getPosition().y);
            if(dist > cfg.params.walkingRange || bin->getType() != BinType::General) reachOk = false;
        }
    }
    check(reachOk, "US31: residents only know General bins within their walking range");
    check(world.bins.back()->getFillLevel() == 0.0, "US31: the recycling bin next to a house was never used");

    // residents of the isolated house (last one) can reach nothing
    // (one resident may make 0 trips in a few days, so the trips are counted for the whole house)
    House* isolated = world.houses.back();
    bool isolatedOk = !isolated->getResidents().empty();
    int isolatedTrips = 0;
    for(const Resident* res : isolated->getResidents()){
        isolatedTrips += res->getTrips();
        if(res->getTotalDisposed() != 0.0 || res->getFailedTrips() != res->getTrips()){
            isolatedOk = false;     // disposed something, or a trip that did not fail
        }
    }
    check(isolatedOk && isolatedTrips > 0, "US31: residents with no bin in reach try, fail and keep their waste");
}


// ---------------------------------------------------------------------
//  US32: randomness and reproducibility (also US42)
// ---------------------------------------------------------------------
void testRandomness(const PopulationConfig& cfg, int days){
    std::cout << "\nUS32 / US42: random behaviour, reproducible with a seed\n";

    World first, second, other;
    runScenario(first, cfg, 42, days);
    runScenario(second, cfg, 42, days);
    runScenario(other, cfg, 43, days);

    check(fingerprint(first) == fingerprint(second), "same seed -> identical results");
    check(fingerprint(first) != fingerprint(other), "different seed -> different results");

    // not everybody goes at the same moment: trips differ between residents
    int fewestTrips = first.residents[0]->getTrips(), mostTrips = fewestTrips;
    for(const Resident* res : first.residents){
        if(res->getTrips() < fewestTrips) fewestTrips = res->getTrips();
        if(res->getTrips() > mostTrips) mostTrips = res->getTrips();
    }
    check(fewestTrips != mostTrips, "US32: residents do not all make the same number of trips");
}


// ---------------------------------------------------------------------
//  US33 + US34: reaction to a full bin depends on the awareness
//
//  Two bins are in reach of one house: bin A is full from the start, bin B
//  never fills up. A trip is made in EVERY tick (disposalsPerDay == ticksPerDay).
//    - awareness 1.0: always tries the other bin -> never fails
//    - awareness 0.0: gives up after the first full bin -> fails whenever it
//      picked bin A first (about half of the trips)
// ---------------------------------------------------------------------
void testFullBinReaction(){
    std::cout << "\nUS33 / US34: reaction to a full bin\n";

    House house(1, Position{0, 0});
    Bin full(1, Position{1, 0}, BinType::General, 10.0);
    Bin open(2, Position{0, 1}, BinType::General, 1e9);
    full.deposit(10.0, WasteType::General);                  // bin A is now full
    std::vector<Bin*> bins = {&full, &open};

    ResidentParams params = {10, 5, 10.0};
    Resident aware(1, Position{0, 0}, 1.0, 1.0, &house, "Aware", params);
    Resident careless(2, Position{0, 0}, 1.0, 0.0, &house, "Careless", params);

    std::mt19937 rng(7);
    for(int tick = 0; tick < 200; tick++){
        aware.step(bins, rng);
        careless.step(bins, rng);
    }

    std::cout << "         aware:    " << aware.getTrips() << " trips, " << aware.getFailedTrips() << " failed\n";
    std::cout << "         careless: " << careless.getTrips() << " trips, " << careless.getFailedTrips() << " failed\n";
    check(aware.getFailedTrips() == 0, "US33/US34: aware resident always finds the free bin");
    check(careless.getFailedTrips() > 0, "US33/US34: careless resident gives up at a full bin");
    check(open.getFillLevel() > 0.0, "US33: waste of a resident that met a full bin ends up in another bin");
}

}   // namespace


int main(){
    const int days = 3;
    PopulationConfig cfg = makeConfig();

    // Scenario A: one run, checked from several points of view
    World world;
    runScenario(world, cfg, 42, days);

    // summary per resident type
    std::cout << "Scenario A: " << cfg.totalResidents << " residents, " << days << " days, seed 42\n";
    std::cout << std::left << std::setw(16) << "  type" << std::setw(11) << "residents"
              << std::setw(12) << "trips" << std::setw(12) << "failed" << "kg disposed\n";
    for(const ResidentType& type : cfg.types){
        int count = 0, trips = 0, failed = 0;
        double disposed = 0.0;
        for(const Resident* res : world.residents){
            if(res->getTypeName() != type.name) continue;
            count++;
            trips += res->getTrips();
            failed += res->getFailedTrips();
            disposed += res->getTotalDisposed();
        }
        std::cout << "  " << std::left << std::setw(14) << type.name << std::setw(11) << count
                  << std::setw(12) << trips << std::setw(12) << failed
                  << std::fixed << std::setprecision(1) << disposed << "\n";
    }

    testPopulation(world, cfg);
    testWaste(world, days);
    testReach(world, cfg);
    testRandomness(cfg, days);
    testFullBinReaction();

    std::cout << "\n" << (failures == 0 ? "All checks passed." : "Some checks FAILED.") << "\n";
    return failures == 0 ? 0 : 1;
}
