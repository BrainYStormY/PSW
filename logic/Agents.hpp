#pragma once

#include "Types.hpp"
#include "Environment.hpp"
#include <list>


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


class Resident{
private:
    int id;
    Position position;
    double wastePerDay;
    double awareness;
    House* house;

public:
// Constructor
    Resident(int id, Position pos, double waste, double aware, House* house) : id(id), position(pos), wastePerDay(waste), awareness(aware), house(house){};
// Actions
    void produceWaste();
    Bin* chooseBin(City& city);
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
