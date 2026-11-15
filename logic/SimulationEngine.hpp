#pragma once

#include "Types.hpp"
#include "Environment.hpp"
#include "Agents.hpp"
#include <vector>
#include <memory>


class Statistics{
private:
    double totalKm = 0.0;
    double fuelUsed = 0.0;
    int overflowingBins = 0;
    double recyclingRate = 0.0;
    double totalCost = 0.0;

    void update(class Simulation& sim);
};


class City{
private:
    int width;
    int height;
    std::vector<Building*> buildings;
    std::vector<Bin*> bins;

public:
// Constructor
    City(int w, int h) : width(w), height(h){};
// Setters
    void addBuilding(Building* building){buildings.push_back(building);}
    void addBin(Bin* bin){bins.push_back(bin);}
    void removeBuilding(Building* building);
    void moveBin(Bin* bin);
};


class Simulation{
private:
    int tick = 0;
    double speed = 1.0;
    int randomSeed;
    bool running = false;
    City city;
    std::vector<Resident*> residents;
    std::vector<Truck*> trucks;
    Statistics stats;

public:
// Constructor
    Simulation(int width, int height, int seed) : city(width, height), randomSeed(seed){};
// Simulation Controls
    void start(){running = true;};
    void pause(){running = false;};
    void resume(){running = true;};
    void reset();
    void setSpeed(double spd){speed = spd;}

    void step(){
        if(!running) return;
        tick ++;

        for(auto& res : residents){res->produceWaste();}
        for(auto& truck : trucks){truck->move();}
        stats.update(*this);
    }
};
