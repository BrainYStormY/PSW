#pragma once

#include "Types.hpp"
#include "Environment.hpp"
#include "PopulationConfig.hpp"     // [Francisco] ResidentParams
#include <list>
#include <random>                   // [Francisco] std::mt19937 (one seeded engine for the whole simulation, US42)
#include <string>                   // [Francisco] name of the resident type
#include <utility>                  // [Francisco] std::move
#include <vector>                   // [Francisco]


class City;


class Route{
private:
    std::list<Bin*> stops;

public:
// Getter
    Bin* nextStop();
// Setter
    void addStop(Bin* bin){stops.push_back(bin);}
};


// ---------------------------------------------------------------------
//  [Francisco] Resident: an agent that lives in a house and produces waste.
//
//  One call of step() is one tick of the agent cycle (perceive -> decide -> act):
//    ACT       produce waste at home                       (US30)
//    DECIDE    go to a bin now? (random)                   (US32)
//    PERCEIVE  which bins are within my walking range?     (US31)
//    DECIDE    which one do I try first? (random)          (US32)
//    ACT       deposit the waste; if the bin is full, react (US33, US34)
//
//  The resident only talks to the world through the bin list that is passed
//  in (agents interact only through the World, see C4 level 2).
//  All randomness comes from the std::mt19937 that is passed in, so a fixed
//  seed gives a reproducible run (US42).
// ---------------------------------------------------------------------
class Resident{
private:
    int id;
    Position position;
    double wastePerDay;         // kg/day (US30)
    double awareness;           // 0..1 (US34)
    House* house;
    // [Francisco] new data members
    std::string typeName;       // which resident type this is (US28), used for statistics per type
    ResidentParams params;      // ticksPerDay, walkingRange, disposalsPerDay
    double carriedWaste = 0.0;  // kg produced but not yet disposed of (waits at home)
    double totalDisposed = 0.0; // kg successfully put into bins since the start
    int trips = 0;              // disposal trips started
    int failedTrips = 0;        // trips that ended without disposing anything

    // [Francisco] decision helpers (private: only step() uses them)
    bool wantsToDispose(std::mt19937& rng) const;                       // US32
    bool reactToFullBin(std::mt19937& rng) const;                       // US33, US34
    void disposeWaste(const std::vector<Bin*>& allBins, std::mt19937& rng);

public:
// Constructor
    // [Francisco] two parameters added at the end (type name, behaviour parameters)
    Resident(int id, Position pos, double waste, double aware, House* house, std::string type, ResidentParams p)
        : id(id), position(pos), wastePerDay(waste), awareness(aware), house(house), typeName(std::move(type)), params(p){};
// Getters  [Francisco]
    int getId() const {return id;}
    Position getPosition() const {return position;}
    double getWastePerDay() const {return wastePerDay;}
    double getAwareness() const {return awareness;}
    House* getHouse() const {return house;}
    const std::string& getTypeName() const {return typeName;}
    double getCarriedWaste() const {return carriedWaste;}
    double getTotalDisposed() const {return totalDisposed;}
    int getTrips() const {return trips;}
    int getFailedTrips() const {return failedTrips;}
// Actions
    void produceWaste();                                                // US30
    // [Francisco] was chooseBin(City&): it now receives the candidate bins directly, because
    // SimulationEngine.hpp (where City is defined) does not compile yet. It removes the chosen
    // bin from the list, so the resident never tries the same bin twice in one trip.
    Bin* chooseBin(std::vector<Bin*>& candidates, std::mt19937& rng) const;   // US32
    std::vector<Bin*> reachableBins(const std::vector<Bin*>& allBins) const;  // US31
    void step(const std::vector<Bin*>& allBins, std::mt19937& rng);           // one tick
};


class Truck{
private:
    int id;
    double speed;
    double capacity;
    double load = 0.0;
    double breakdownProb;
    TruckState state = TruckState::Idle;
    Route route;
    Depot* startingDepot;

public:
// Constructor
    Truck(int id, double speed, double cap, double breakProb, Depot* depot) : id(id), speed(speed), capacity(cap), breakdownProb(breakProb), startingDepot(depot){};
// Actions
    void move();
    void collect(Bin* bin);
    void unload(Building* facility);    // might be a Landfill or a RecyclingCenter
};
