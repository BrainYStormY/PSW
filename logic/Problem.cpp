// =====================================================================
//  Problem.cpp  -  Helpers shared by all routing algorithms
// =====================================================================

#include "Problem.hpp"
#include "RoutePlanner.hpp"
#include <climits>     // INT_MAX


double RouteUtils::load(const std::vector<Bin*>& stops){
    double total = 0.0;
    for(Bin* b : stops) total += b->getFillLevel();
    return total;
}


int RouteUtils::length(const Problem& problem, const TruckRoute& route){
    if(route.stops.empty()) return 0;   // unused truck: stays where it is
    RoutePlanner planner(problem.trucks[route.truck].start, problem.depot);
    return planner.totalDistance(route.stops);
}


int RouteUtils::totalDistance(const Problem& problem, const Plan& plan){
    int total = 0;
    for(const TruckRoute& r : plan.routes) total += length(problem, r);
    return total;
}


int RouteUtils::trucksUsed(const Plan& plan){
    int used = 0;
    for(const TruckRoute& r : plan.routes){
        if(!r.stops.empty()) used++;
    }
    return used;
}


Plan RouteUtils::emptyPlan(const Problem& problem, const std::string& name){
    Plan plan;
    plan.algorithm = name;
    for(size_t i = 0; i < problem.trucks.size(); i++){
        plan.routes.push_back(TruckRoute{(int)i, {}});
    }
    return plan;
}


// ---------------------------------------------------------------------
//  Cheapest insertion of one bin in one route
// ---------------------------------------------------------------------
//  Putting bin B between stops P and N changes the route like this:
//      before:  P -> N
//      after:   P -> B -> N
//      extra = d(P,B) + d(B,N) - d(P,N)
//  We test every position and keep the one with the smallest extra.
//  An empty truck has length 0, so its extra is the full trip
//  start -> B -> depot (opening a new truck is expensive).
// ---------------------------------------------------------------------
int RouteUtils::bestInsertion(const Problem& problem, const TruckRoute& route, Bin* bin, size_t& bestPos){
    const TruckInfo& truck = problem.trucks[route.truck];
    if(load(route.stops) + bin->getFillLevel() > truck.capacity) return INT_MAX;   // no room

    Position b = bin->getPosition();
    if(route.stops.empty()){
        bestPos = 0;
        return RoutePlanner::distance(truck.start, b) + RoutePlanner::distance(b, problem.depot);
    }

    int best = INT_MAX;
    for(size_t pos = 0; pos <= route.stops.size(); pos++){
        // stop before and after the new bin
        Position prev = (pos == 0) ? truck.start : route.stops[pos-1]->getPosition();
        Position next = (pos == route.stops.size()) ? problem.depot : route.stops[pos]->getPosition();

        int extra = RoutePlanner::distance(prev, b) + RoutePlanner::distance(b, next)
                  - RoutePlanner::distance(prev, next);
        if(extra < best){
            best = extra;
            bestPos = pos;
        }
    }
    return best;
}


bool RouteUtils::insertCheapest(const Problem& problem, Plan& plan, Bin* bin){
    int best = INT_MAX;
    size_t bestRoute = 0, bestPos = 0;

    for(size_t r = 0; r < plan.routes.size(); r++){
        size_t pos = 0;
        int extra = bestInsertion(problem, plan.routes[r], bin, pos);
        if(extra < best){
            best = extra;
            bestRoute = r;
            bestPos = pos;
        }
    }
    if(best == INT_MAX) return false;   // no truck has room

    std::vector<Bin*>& stops = plan.routes[bestRoute].stops;
    stops.insert(stops.begin() + bestPos, bin);
    return true;
}


void RouteUtils::insertUnassigned(const Problem& problem, Plan& plan){
    std::vector<Bin*> stillOut;
    for(Bin* b : plan.unassigned){
        if(!insertCheapest(problem, plan, b)) stillOut.push_back(b);
    }
    plan.unassigned = stillOut;
}


bool RouteUtils::isBetter(const Problem& problem, const Plan& a, const Plan& b){
    if(a.unassigned.size() != b.unassigned.size()){
        return a.unassigned.size() < b.unassigned.size();
    }
    return totalDistance(problem, a) < totalDistance(problem, b);
}
