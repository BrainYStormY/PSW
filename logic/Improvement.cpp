// =====================================================================
//  Improvement.cpp  -  2-opt + relocate on a finished plan
// =====================================================================

#include "Improvement.hpp"
#include "RoutePlanner.hpp"
#include <climits>     // INT_MAX


// ---------------------------------------------------------------------
//  Relocate: take a bin out of its truck and put it in another truck.
//    gain  = how much shorter its old route gets without it
//    extra = how much longer the new route gets with it
//  If extra < gain, the total distance goes down -> do the move.
//  Returns true after the first good move (the caller repeats).
// ---------------------------------------------------------------------
static bool relocateOnce(const Problem& problem, Plan& plan){
    for(size_t a = 0; a < plan.routes.size(); a++){
        for(size_t i = 0; i < plan.routes[a].stops.size(); i++){
            Bin* bin = plan.routes[a].stops[i];

            TruckRoute without = plan.routes[a];
            without.stops.erase(without.stops.begin() + i);
            int gain = RouteUtils::length(problem, plan.routes[a]) - RouteUtils::length(problem, without);

            for(size_t c = 0; c < plan.routes.size(); c++){
                if(c == a) continue;
                size_t pos = 0;
                int extra = RouteUtils::bestInsertion(problem, plan.routes[c], bin, pos);
                if(extra != INT_MAX && extra < gain){
                    plan.routes[a] = without;
                    std::vector<Bin*>& stops = plan.routes[c].stops;
                    stops.insert(stops.begin() + pos, bin);
                    return true;
                }
            }
        }
    }
    return false;
}


void Improvement::apply(const Problem& problem, Plan& plan){
    // each loop makes the distance strictly smaller, so it always ends
    while(true){
        // 1) 2-opt inside each route
        for(TruckRoute& r : plan.routes){
            if(r.stops.size() < 2) continue;
            RoutePlanner planner(problem.trucks[r.truck].start, problem.depot);
            planner.twoOpt(r.stops);
        }

        // bins left out before may fit now
        RouteUtils::insertUnassigned(problem, plan);

        // 2) relocate between trucks
        if(!relocateOnce(problem, plan)) break;
    }
}
