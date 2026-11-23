#pragma once

#include "Types.hpp"
#include "Environment.hpp"
#include <vector>


// Plans the route of ONE truck: start -> bins -> end.
// Method: nearest neighbour gives a first order, then 2-opt improves it.
class RoutePlanner{
private:
    Position start;   // where the truck leaves from
    Position end;     // where the truck must finish (the depot)

    // Total length: start -> bins in the given order -> end
    int routeLength(const std::vector<Bin*>& order) const;

    // Step 1: nearest neighbour
    std::vector<Bin*> nearestNeighbour(const std::vector<Bin*>& bins) const;

public:
// Constructors
    // Old version: the truck starts and ends at the depot
    RoutePlanner(Position depotPos) : start(depotPos), end(depotPos){};
    // New version: the truck starts at startPos and ends at endPos
    RoutePlanner(Position startPos, Position endPos) : start(startPos), end(endPos){};

// Grid distance with no obstacles (the truck only moves horizontally/vertically)
    static int distance(Position a, Position b);

// Returns the order in which the truck should visit the bins
    std::vector<Bin*> planOrder(const std::vector<Bin*>& bins) const;

// Step 2: improves an existing order with 2-opt (public so other algorithms can use it)
    void twoOpt(std::vector<Bin*>& order) const;

// Returns every grid cell the truck goes through (full trajectory)
    std::vector<Position> buildPath(const std::vector<Bin*>& order) const;

    int totalDistance(const std::vector<Bin*>& order) const {return routeLength(order);}
};
