// =====================================================================
//  InsertionAlgorithm.cpp  -  Cheapest insertion
// ---------------------------------------------------------------------
//  Each step tests every (bin, truck, position) still possible and
//  does the one with the smallest extra distance (see bestInsertion
//  in Problem.cpp). Stops when every bin is placed or nothing fits.
// =====================================================================

#include "InsertionAlgorithm.hpp"
#include <climits>     // INT_MAX


Plan InsertionAlgorithm::solve(const Problem& problem) const{
    Plan plan = RouteUtils::emptyPlan(problem, name());
    std::vector<Bin*> remaining = problem.bins;

    while(!remaining.empty()){
        int best = INT_MAX;
        size_t bestBin = 0, bestRoute = 0, bestPos = 0;

        for(size_t i = 0; i < remaining.size(); i++){
            for(size_t r = 0; r < plan.routes.size(); r++){
                size_t pos = 0;
                int extra = RouteUtils::bestInsertion(problem, plan.routes[r], remaining[i], pos);
                if(extra < best){
                    best = extra;
                    bestBin = i;
                    bestRoute = r;
                    bestPos = pos;
                }
            }
        }
        if(best == INT_MAX) break;   // no bin fits in any truck

        std::vector<Bin*>& stops = plan.routes[bestRoute].stops;
        stops.insert(stops.begin() + bestPos, remaining[bestBin]);
        remaining.erase(remaining.begin() + bestBin);
    }

    plan.unassigned = remaining;
    return plan;
}
