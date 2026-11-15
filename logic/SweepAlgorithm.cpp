// =====================================================================
//  SweepAlgorithm.cpp  -  Split bins into angle slices, one per truck
// =====================================================================

#include "SweepAlgorithm.hpp"
#include "RoutePlanner.hpp"
#include <cmath>       // std::atan2, std::acos
#include <algorithm>   // std::stable_sort, std::rotate
#include <utility>     // std::pair


Plan SweepAlgorithm::solve(const Problem& problem) const{
    Plan plan = RouteUtils::emptyPlan(problem, name());
    if(problem.bins.empty()) return plan;

    // 1) Angle of each bin around the depot (from -pi to pi)
    std::vector<std::pair<double, Bin*>> sorted;
    for(Bin* b : problem.bins){
        Position p = b->getPosition();
        double angle = std::atan2(double(p.y - problem.depot.y), double(p.x - problem.depot.x));
        sorted.push_back({angle, b});
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const auto& a, const auto& b){ return a.first < b.first; });

    // 2) Start the sweep right after the biggest empty angle,
    //    so a group of nearby bins is not cut in two.
    const double TWO_PI = 2.0 * std::acos(-1.0);
    size_t n = sorted.size();
    size_t startIndex = 0;
    double biggestGap = -1.0;
    for(size_t i = 0; i < n; i++){
        double next = (i + 1 < n) ? sorted[i+1].first : sorted[0].first + TWO_PI;
        double gap = next - sorted[i].first;
        if(gap > biggestGap){
            biggestGap = gap;
            startIndex = (i + 1) % n;
        }
    }
    std::rotate(sorted.begin(), sorted.begin() + startIndex, sorted.end());

    // 3) Fill the trucks one at a time, in angle order
    size_t t = 0;          // current truck
    double load = 0.0;     // waste already in the current truck
    for(const auto& item : sorted){
        Bin* bin = item.second;
        double waste = bin->getFillLevel();

        // current truck is full -> move to the next one
        if(t < problem.trucks.size() && load + waste > problem.trucks[t].capacity
           && !plan.routes[t].stops.empty()){
            t++;
            load = 0.0;
        }
        // no truck left, or the bin is bigger than the truck
        if(t >= problem.trucks.size() || load + waste > problem.trucks[t].capacity){
            plan.unassigned.push_back(bin);
            continue;
        }
        plan.routes[t].stops.push_back(bin);
        load += waste;
    }

    // 4) Bins left out may still fit in the free space of some truck
    RouteUtils::insertUnassigned(problem, plan);

    // 5) Order of each truck: the single-truck planner
    for(TruckRoute& r : plan.routes){
        if(r.stops.empty()) continue;
        RoutePlanner planner(problem.trucks[r.truck].start, problem.depot);
        r.stops = planner.planOrder(r.stops);
    }
    return plan;
}
