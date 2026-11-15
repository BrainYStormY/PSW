// =====================================================================
//  ClarkeWrightAlgorithm.cpp  -  Savings algorithm
// ---------------------------------------------------------------------
//  Two separate trips   D -> i -> D   and   D -> j -> D
//  cost  2*d(D,i) + 2*d(D,j).
//  One joined trip      D -> i -> j -> D
//  costs d(D,i) + d(i,j) + d(j,D).
//  Saving of joining = d(D,i) + d(D,j) - d(i,j).
//
//  We join routes from the biggest saving to the smallest, only when:
//    - i and j are in different routes,
//    - i is at an end of its route and j at an end of its route
//      (we can only glue routes by their ends),
//    - the joined load fits in the biggest truck.
//
//  Note: savings use the depot as start. If a truck starts somewhere
//  else, we still use this and only pick the best direction at the end.
// =====================================================================

#include "ClarkeWrightAlgorithm.hpp"
#include "RoutePlanner.hpp"
#include <algorithm>   // std::sort, std::reverse


Plan ClarkeWrightAlgorithm::solve(const Problem& problem) const{
    Plan plan = RouteUtils::emptyPlan(problem, name());
    const std::vector<Bin*>& bins = problem.bins;
    size_t n = bins.size();
    if(n == 0) return plan;

    double maxCapacity = 0.0;
    for(const TruckInfo& t : problem.trucks) maxCapacity = std::max(maxCapacity, t.capacity);

    // 1) One route per bin (routes hold bin indexes)
    std::vector<std::vector<size_t>> routes(n);
    std::vector<double> loads(n);
    std::vector<size_t> routeOf(n);     // routeOf[i] -> route that has bin i
    for(size_t i = 0; i < n; i++){
        routes[i].push_back(i);
        loads[i] = bins[i]->getFillLevel();
        routeOf[i] = i;
    }

    // 2) Saving of every pair of bins, biggest first
    struct Saving{ size_t i, j; int value; };
    std::vector<Saving> savings;
    Position D = problem.depot;
    for(size_t i = 0; i < n; i++){
        for(size_t j = i + 1; j < n; j++){
            Position pi = bins[i]->getPosition(), pj = bins[j]->getPosition();
            int value = RoutePlanner::distance(D, pi) + RoutePlanner::distance(D, pj)
                      - RoutePlanner::distance(pi, pj);
            savings.push_back({i, j, value});
        }
    }
    std::stable_sort(savings.begin(), savings.end(),
                     [](const Saving& a, const Saving& b){ return a.value > b.value; });

    // 3) Join routes
    for(const Saving& s : savings){
        size_t ri = routeOf[s.i], rj = routeOf[s.j];
        if(ri == rj) continue;                                   // already together
        if(loads[ri] + loads[rj] > maxCapacity) continue;        // too heavy

        std::vector<size_t>& A = routes[ri];
        std::vector<size_t>& B = routes[rj];
        bool iAtEnd = (A.front() == s.i || A.back() == s.i);
        bool jAtEnd = (B.front() == s.j || B.back() == s.j);
        if(!iAtEnd || !jAtEnd) continue;                         // i or j in the middle

        // turn the routes so that A ends with i and B starts with j, then glue: ... i -> j ...
        if(A.back() != s.i) std::reverse(A.begin(), A.end());
        if(B.front() != s.j) std::reverse(B.begin(), B.end());
        for(size_t b : B){
            A.push_back(b);
            routeOf[b] = ri;
        }
        B.clear();
        loads[ri] += loads[rj];
        loads[rj] = 0.0;
    }

    // 4) Give routes to trucks: heaviest route first, to the smallest truck where it fits
    std::vector<size_t> order;
    for(size_t r = 0; r < n; r++){
        if(!routes[r].empty()) order.push_back(r);
    }
    std::stable_sort(order.begin(), order.end(),
                     [&](size_t a, size_t b){ return loads[a] > loads[b]; });

    std::vector<bool> used(problem.trucks.size(), false);
    for(size_t r : order){
        int bestTruck = -1;
        for(size_t t = 0; t < problem.trucks.size(); t++){
            if(used[t] || problem.trucks[t].capacity < loads[r]) continue;
            if(bestTruck == -1 || problem.trucks[t].capacity < problem.trucks[bestTruck].capacity){
                bestTruck = t;
            }
        }

        if(bestTruck == -1){
            // more routes than trucks: these bins are placed one by one below
            for(size_t b : routes[r]) plan.unassigned.push_back(bins[b]);
            continue;
        }
        used[bestTruck] = true;

        // pick the direction that is shorter for this truck's start
        TruckRoute forward{bestTruck, {}};
        for(size_t b : routes[r]) forward.stops.push_back(bins[b]);
        TruckRoute backward = forward;
        std::reverse(backward.stops.begin(), backward.stops.end());

        if(RouteUtils::length(problem, backward) < RouteUtils::length(problem, forward)){
            plan.routes[bestTruck] = backward;
        }
        else{
            plan.routes[bestTruck] = forward;
        }
    }

    // 5) Bins without a truck go where they add the least distance
    RouteUtils::insertUnassigned(problem, plan);
    return plan;
}
