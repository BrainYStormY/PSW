// =====================================================================
//  RoutePlanner.cpp  -  Route of ONE truck
//  (see the general idea in RoutePlanner.hpp)
// =====================================================================

#include "RoutePlanner.hpp"
#include <cstdlib>     // std::abs
#include <algorithm>   // std::reverse
#include <climits>     // INT_MAX


// Manhattan distance: cells to move horizontally + cells to move vertically.
// Ex: (0,0) -> (3,4) = 3 + 4 = 7 cells.
int RoutePlanner::distance(Position a, Position b){
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}


// Sum of every leg: start -> 1st bin -> ... -> last bin -> end.
// With no bins the truck just goes from start to end.
int RoutePlanner::routeLength(const std::vector<Bin*>& order) const{
    if(order.empty()) return distance(start, end);

    int total = distance(start, order.front()->getPosition());                  // start -> 1st bin
    for(size_t i = 1; i < order.size(); i++){
        total += distance(order[i-1]->getPosition(), order[i]->getPosition());  // bin -> bin
    }
    total += distance(order.back()->getPosition(), end);                        // last bin -> end
    return total;
}


// ---------------------------------------------------------------------
//  STEP 1 - Nearest neighbour
// ---------------------------------------------------------------------
//  Start at the truck position. At each step go to the closest bin
//  that was not visited yet. Repeat until every bin is visited.
//  Fast and simple, but "greedy" (it only looks one step ahead), so
//  the route can have crossings. Step 2 fixes that.
// ---------------------------------------------------------------------
std::vector<Bin*> RoutePlanner::nearestNeighbour(const std::vector<Bin*>& bins) const{
    std::vector<Bin*> order;
    std::vector<bool> visited(bins.size(), false);   // visited[i] -> bin i already chosen?
    Position current = start;

    for(size_t step = 0; step < bins.size(); step++){
        int best = -1;           // index of the closest bin found so far
        int bestDist = INT_MAX;  // start with "infinity" so any distance is smaller

        for(size_t i = 0; i < bins.size(); i++){
            if(visited[i]) continue;
            int d = distance(current, bins[i]->getPosition());
            if(d < bestDist){
                bestDist = d;
                best = i;
            }
        }

        // go to the closest one
        visited[best] = true;
        order.push_back(bins[best]);
        current = bins[best]->getPosition();
    }
    return order;
}


// ---------------------------------------------------------------------
//  STEP 2 - 2-opt improvement
// ---------------------------------------------------------------------
//  Idea: pick a piece of the route (from bin i to bin j) and travel
//  it backwards. Example with order A B C D E:
//      reverse B..D  ->  A D C B E
//  This removes a "crossing" and makes the route shorter.
//
//  Every pair (i, j) is tested:
//    - if the route gets shorter, keep the change;
//    - if not, reverse again to undo it.
//  Repeat while something improves.
//
//  Note: start and end are not in the vector, so they never move;
//  routeLength already counts them.
// ---------------------------------------------------------------------
void RoutePlanner::twoOpt(std::vector<Bin*>& order) const{
    bool improved = true;

    while(improved){
        improved = false;
        int bestLength = routeLength(order);

        for(size_t i = 0; i + 1 < order.size(); i++){
            for(size_t j = i + 1; j < order.size(); j++){
                // reverse the piece between i and j (inclusive)
                std::reverse(order.begin() + i, order.begin() + j + 1);
                int newLength = routeLength(order);

                if(newLength < bestLength){
                    bestLength = newLength;   // better: keep it
                    improved = true;
                }
                else{
                    std::reverse(order.begin() + i, order.begin() + j + 1);   // not better: undo
                }
            }
        }
    }
}


// Both steps together: first order + improvement
std::vector<Bin*> RoutePlanner::planOrder(const std::vector<Bin*>& bins) const{
    std::vector<Bin*> order = nearestNeighbour(bins);
    twoOpt(order);
    return order;
}


// ---------------------------------------------------------------------
//  Cell by cell trajectory
// ---------------------------------------------------------------------
//  The order says WHICH bins and in what order; this says WHERE the
//  truck drives. Between two stops it first moves all the way in x,
//  then all the way in y ("L" shape). Any path with only horizontal/
//  vertical steps in the right direction has the same Manhattan
//  length, so this choice does not change the total distance.
// ---------------------------------------------------------------------
std::vector<Position> RoutePlanner::buildPath(const std::vector<Bin*>& order) const{
    std::vector<Position> path;
    Position current = start;
    path.push_back(current);

    // targets in order: the bins and, at the end, the end point
    std::vector<Position> targets;
    for(Bin* bin : order) targets.push_back(bin->getPosition());
    targets.push_back(end);

    for(Position target : targets){
        // move one cell at a time in x until the column is right...
        while(current.x != target.x){
            current.x += (target.x > current.x) ? 1 : -1;
            path.push_back(current);
        }
        // ...then in y until the row is right
        while(current.y != target.y){
            current.y += (target.y > current.y) ? 1 : -1;
            path.push_back(current);
        }
    }
    return path;
}
