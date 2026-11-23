#pragma once

#include "Types.hpp"
#include "Environment.hpp"
#include <vector>
#include <string>


// Planning data for one truck.
// Kept separate from the Truck class (Agents.hpp) so that class does not change.
struct TruckInfo{
    int id;
    Position start;     // where the truck is when planning starts
    double capacity;    // max waste it can carry
};


// Everything the routing algorithms need to know.
// The Problem owns the bins (created with new) and deletes them at the end.
struct Problem{
    int gridSize = 30;
    Position depot{0, 0};          // every route ends here
    std::vector<TruckInfo> trucks;
    std::vector<Bin*> bins;

    Problem() = default;
    ~Problem(){ for(Bin* b : bins) delete b; }
    Problem(const Problem&) = delete;              // no copies (it owns the bins)
    Problem& operator=(const Problem&) = delete;
};


// Route of one truck: truck start -> stops -> depot
struct TruckRoute{
    int truck;                     // index in Problem::trucks
    std::vector<Bin*> stops;       // bins in visiting order
};


// Result of an algorithm: one route per truck + bins that did not fit
struct Plan{
    std::string algorithm;
    std::vector<TruckRoute> routes;
    std::vector<Bin*> unassigned;
};


// Small helpers used by every algorithm
namespace RouteUtils{
    // Total waste of a list of bins
    double load(const std::vector<Bin*>& stops);

    // Length of one route (an unused truck counts as 0: it does not leave)
    int length(const Problem& problem, const TruckRoute& route);

    // Cost of a plan = total distance of all trucks
    int totalDistance(const Problem& problem, const Plan& plan);

    // How many trucks have at least one bin
    int trucksUsed(const Plan& plan);

    // A plan with one empty route per truck
    Plan emptyPlan(const Problem& problem, const std::string& name);

    // Extra distance to put 'bin' in the best place of 'route'.
    // Writes the best position in 'bestPos'. Returns INT_MAX if it does not fit.
    int bestInsertion(const Problem& problem, const TruckRoute& route, Bin* bin, size_t& bestPos);

    // Puts the bin where it adds the least distance, in any truck with room.
    // Returns false if no truck has room.
    bool insertCheapest(const Problem& problem, Plan& plan, Bin* bin);

    // Tries to place every unassigned bin; the ones that still do not fit stay unassigned
    void insertUnassigned(const Problem& problem, Plan& plan);

    // Is plan a better than plan b? (fewer unassigned bins first, then shorter distance)
    bool isBetter(const Problem& problem, const Plan& a, const Plan& b);
}
