// =====================================================================
//  KMeansAlgorithm.cpp  -  Group bins by proximity, one group per truck
// ---------------------------------------------------------------------
//  Classic k-means repeats two steps:
//    a) give each bin to the closest group centre;
//    b) move each centre to the middle of its bins.
//  Here step a) also respects capacity: if the closest group is full,
//  the bin goes to the next closest one with room.
//  It stops when no bin changes group (or after 100 rounds).
// =====================================================================

#include "KMeansAlgorithm.hpp"
#include "RoutePlanner.hpp"
#include <cmath>       // std::fabs
#include <algorithm>   // std::sort, std::min
#include <numeric>     // std::iota
#include <limits>


namespace{
    // Group centre (can be between cells, so it uses double)
    struct Centre{ double x, y; };

    double dist(Centre c, Position p){
        return std::fabs(c.x - p.x) + std::fabs(c.y - p.y);   // Manhattan, like the trucks
    }
}


Plan KMeansAlgorithm::solve(const Problem& problem) const{
    Plan plan = RouteUtils::emptyPlan(problem, name());
    const std::vector<Bin*>& bins = problem.bins;
    size_t n = bins.size();
    size_t k = std::min(problem.trucks.size(), n);   // never more groups than bins
    if(k == 0){
        plan.unassigned = bins;
        return plan;
    }

    // 1) First centres: the bin farthest from the depot, then each time
    //    the bin farthest from the centres already chosen (spreads them out).
    std::vector<Centre> centres;
    std::vector<double> closest(n);   // distance from each bin to its closest centre
    size_t first = 0;
    for(size_t i = 0; i < n; i++){
        if(RoutePlanner::distance(problem.depot, bins[i]->getPosition()) >
           RoutePlanner::distance(problem.depot, bins[first]->getPosition())) first = i;
    }
    centres.push_back({double(bins[first]->getPosition().x), double(bins[first]->getPosition().y)});
    for(size_t i = 0; i < n; i++) closest[i] = dist(centres[0], bins[i]->getPosition());

    while(centres.size() < k){
        size_t far = 0;
        for(size_t i = 0; i < n; i++){
            if(closest[i] > closest[far]) far = i;
        }
        Centre c{double(bins[far]->getPosition().x), double(bins[far]->getPosition().y)};
        centres.push_back(c);
        for(size_t i = 0; i < n; i++) closest[i] = std::min(closest[i], dist(c, bins[i]->getPosition()));
    }

    // 2) Repeat: assign bins (with capacity) and move the centres
    std::vector<int> group(n, -2);   // group of each bin (-1 = no room anywhere)
    for(int round = 0; round < 100; round++){
        // bins closest to a centre choose first
        std::vector<double> best(n, std::numeric_limits<double>::max());
        for(size_t i = 0; i < n; i++){
            for(const Centre& c : centres) best[i] = std::min(best[i], dist(c, bins[i]->getPosition()));
        }
        std::vector<size_t> order(n);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](size_t a, size_t b){ return best[a] < best[b]; });

        // free space in each group = capacity of its truck
        std::vector<double> room(k);
        for(size_t g = 0; g < k; g++) room[g] = problem.trucks[g].capacity;

        std::vector<int> newGroup(n, -1);
        for(size_t i : order){
            Position p = bins[i]->getPosition();
            // groups from closest to farthest
            std::vector<size_t> groups(k);
            std::iota(groups.begin(), groups.end(), 0);
            std::sort(groups.begin(), groups.end(),
                      [&](size_t a, size_t b){ return dist(centres[a], p) < dist(centres[b], p); });

            for(size_t g : groups){
                if(bins[i]->getFillLevel() <= room[g]){
                    newGroup[i] = g;
                    room[g] -= bins[i]->getFillLevel();
                    break;
                }
            }
        }

        if(newGroup == group) break;   // nothing changed -> done
        group = newGroup;

        // move each centre to the average position of its bins
        for(size_t g = 0; g < k; g++){
            double sx = 0, sy = 0;
            int count = 0;
            for(size_t i = 0; i < n; i++){
                if(group[i] == (int)g){
                    sx += bins[i]->getPosition().x;
                    sy += bins[i]->getPosition().y;
                    count++;
                }
            }
            if(count > 0) centres[g] = {sx / count, sy / count};   // empty group keeps its centre
        }
    }

    // 3) Group g goes to truck g
    for(size_t i = 0; i < n; i++){
        if(group[i] >= 0) plan.routes[group[i]].stops.push_back(bins[i]);
        else plan.unassigned.push_back(bins[i]);
    }
    RouteUtils::insertUnassigned(problem, plan);

    // 4) Order of each truck: the single-truck planner
    for(TruckRoute& r : plan.routes){
        if(r.stops.empty()) continue;
        RoutePlanner planner(problem.trucks[r.truck].start, problem.depot);
        r.stops = planner.planOrder(r.stops);
    }
    return plan;
}
