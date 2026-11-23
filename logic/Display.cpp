// =====================================================================
//  Display.cpp  -  Text output: routes, maps and comparison table
// =====================================================================

#include "Display.hpp"
#include "RoutePlanner.hpp"
#include <iostream>
#include <iomanip>     // std::setw
#include <string>
#include <cstdlib>     // std::abs
#include <cctype>      // std::tolower


namespace{
    // Truck 0 -> 'A', truck 1 -> 'B', ...
    char truckLetter(int index){
        return (index >= 0 && index < 26) ? char('A' + index) : '?';
    }

    std::string pos(Position p){
        return "(" + std::to_string(p.x) + "," + std::to_string(p.y) + ")";
    }

    std::string binName(Bin* b){
        return "C" + std::to_string(b->getId());
    }

    // Prints a grid with x on top and y on the left.
    // On screen y GROWS DOWNWARDS (row 0 is the top one).
    void printGrid(const std::vector<std::vector<char>>& grid){
        int size = grid.size();
        std::cout << "    ";
        for(int x = 0; x < size; x++) std::cout << x % 10 << ' ';   // only the last digit, so columns line up
        std::cout << "  <- x\n";

        for(int y = 0; y < size; y++){
            std::cout << (y < 10 ? " " : "") << y << "  ";
            for(int x = 0; x < size; x++) std::cout << grid[y][x] << ' ';
            std::cout << '\n';
        }
    }


    // -----------------------------------------------------------------
    //  Map of ONE truck with arrows
    // -----------------------------------------------------------------
    //    '.'          cell the truck does not use
    //    > < ^ v      direction the truck LEAVES that cell
    //    1, 2, 3...   bins, numbered by collection ORDER (not by id)
    //    'B'          bin from the 10th on (only one digit per cell)
    //    'S'          truck start (only if it is not the depot)
    //    'D'          depot
    //  Draw order matters: arrows first, then bins, then S and D,
    //  so these are not hidden by arrows.
    // -----------------------------------------------------------------
    void printArrowMap(int size, Position start, Position depot,
                       const std::vector<Bin*>& order, const std::vector<Position>& path){
        std::vector<std::vector<char>> grid(size, std::vector<char>(size, '.'));

        // compare each cell of the path with the next one to get the direction
        for(size_t i = 0; i + 1 < path.size(); i++){
            Position a = path[i], b = path[i+1];
            char arrow;
            if(b.x > a.x)      arrow = '>';
            else if(b.x < a.x) arrow = '<';
            else if(b.y > a.y) arrow = 'v';    // y grows downwards on screen
            else               arrow = '^';
            grid[a.y][a.x] = arrow;            // if the truck passes twice, the last arrow stays
        }

        for(size_t i = 0; i < order.size(); i++){
            Position p = order[i]->getPosition();
            grid[p.y][p.x] = (i < 9) ? char('1' + i) : 'B';
        }
        if(!(start == depot)) grid[start.y][start.x] = 'S';
        grid[depot.y][depot.x] = 'D';

        printGrid(grid);
    }


    // -----------------------------------------------------------------
    //  Step by step of ONE truck
    // -----------------------------------------------------------------
    //  dx = target.x - current.x  (positive = right, negative = left)
    //  dy = target.y - current.y  (positive = down,  negative = up)
    //  |dx| + |dy| is the Manhattan distance of that leg.
    // -----------------------------------------------------------------
    void printSteps(Position start, Position depot, const std::vector<Bin*>& order){
        // stops in order, ending at the depot
        std::vector<Position> stops;
        std::vector<std::string> names;
        for(Bin* b : order){
            stops.push_back(b->getPosition());
            names.push_back(binName(b));
        }
        stops.push_back(depot);
        names.push_back("D");

        Position current = start;
        std::string currentName = (start == depot) ? "D" : "S";
        for(size_t i = 0; i < stops.size(); i++){
            Position t = stops[i];
            int dx = t.x - current.x, dy = t.y - current.y;

            std::cout << "  " << i+1 << ". " << currentName << pos(current) << " -> "
                      << names[i] << pos(t) << ": ";
            if(dx > 0) std::cout << dx << " right  ";
            if(dx < 0) std::cout << -dx << " left  ";
            if(dy > 0) std::cout << dy << " down  ";
            if(dy < 0) std::cout << -dy << " up  ";
            std::cout << "[" << std::abs(dx) + std::abs(dy) << " cells]\n";

            // the target becomes the start of the next leg
            current = t;
            currentName = names[i];
        }
    }
}


void Display::printProblem(const Problem& problem){
    double waste = 0.0, capacity = 0.0;
    for(Bin* b : problem.bins) waste += b->getFillLevel();
    for(const TruckInfo& t : problem.trucks) capacity += t.capacity;

    std::cout << "Grid " << problem.gridSize << "x" << problem.gridSize
              << " | depot " << pos(problem.depot)
              << " | " << problem.trucks.size() << " trucks"
              << " | " << problem.bins.size() << " bins\n";
    std::cout << "Total waste: " << waste << " | total truck capacity: " << capacity << "\n";
}


void Display::printPlan(const Problem& problem, const Plan& plan){
    std::cout << "\n===== " << plan.algorithm << " =====\n";

    for(const TruckRoute& r : plan.routes){
        const TruckInfo& t = problem.trucks[r.truck];
        std::cout << "Truck " << truckLetter(r.truck) << " (id " << t.id << "): ";
        if(r.stops.empty()){
            std::cout << "not used\n";
            continue;
        }

        std::cout << ((t.start == problem.depot) ? "D" : "S") << pos(t.start);
        for(Bin* b : r.stops) std::cout << " -> " << binName(b) << pos(b->getPosition());
        std::cout << " -> D" << pos(problem.depot)
                  << "  | load " << RouteUtils::load(r.stops) << "/" << t.capacity
                  << " | " << RouteUtils::length(problem, r) << " cells\n";
    }

    if(!plan.unassigned.empty()){
        std::cout << "Unassigned (no truck has room):";
        for(Bin* b : plan.unassigned) std::cout << " " << binName(b);
        std::cout << "\n";
    }
    std::cout << "Total distance: " << RouteUtils::totalDistance(problem, plan) << " cells\n";
}


// ---------------------------------------------------------------------
//  Map with every truck
// ---------------------------------------------------------------------
//    a, b, c...   path of truck A, B, C...
//    A, B, C...   bin collected by truck A, B, C...
//    X            unassigned bin
//    S            truck start (only if it is not the depot)
//    D            depot
//  Where paths of two trucks cross, the last truck drawn is shown.
// ---------------------------------------------------------------------
void Display::printOverviewMap(const Problem& problem, const Plan& plan){
    int size = problem.gridSize;
    std::vector<std::vector<char>> grid(size, std::vector<char>(size, '.'));

    for(const TruckRoute& r : plan.routes){
        if(r.stops.empty()) continue;
        RoutePlanner planner(problem.trucks[r.truck].start, problem.depot);
        for(Position p : planner.buildPath(r.stops)){
            grid[p.y][p.x] = std::tolower(truckLetter(r.truck));
        }
    }
    for(const TruckRoute& r : plan.routes){
        for(Bin* b : r.stops) grid[b->getPosition().y][b->getPosition().x] = truckLetter(r.truck);
    }
    for(Bin* b : plan.unassigned) grid[b->getPosition().y][b->getPosition().x] = 'X';
    for(const TruckRoute& r : plan.routes){
        Position s = problem.trucks[r.truck].start;
        if(!r.stops.empty() && !(s == problem.depot)) grid[s.y][s.x] = 'S';
    }
    grid[problem.depot.y][problem.depot.x] = 'D';

    std::cout << "\nMap (D = depot, A/B/C = bins of each truck, a/b/c = path, X = unassigned):\n";
    printGrid(grid);
}


void Display::printTruckDetails(const Problem& problem, const Plan& plan){
    for(const TruckRoute& r : plan.routes){
        if(r.stops.empty()) continue;
        const TruckInfo& t = problem.trucks[r.truck];
        RoutePlanner planner(t.start, problem.depot);

        std::cout << "\n--- Truck " << truckLetter(r.truck) << " (id " << t.id << ") ---\n";
        printSteps(t.start, problem.depot, r.stops);
        std::cout << "\n  Map (D = depot, 1,2,3... = collection order, arrows = direction):\n";
        printArrowMap(problem.gridSize, t.start, problem.depot, r.stops, planner.buildPath(r.stops));
    }
}


void Display::printComparison(const Problem& problem, const std::vector<Plan>& plans, size_t best){
    std::cout << "\n" << std::left
              << std::setw(30) << "Algorithm"
              << std::setw(10) << "Trucks"
              << std::setw(13) << "Unassigned"
              << "Distance\n";
    for(size_t i = 0; i < plans.size(); i++){
        std::cout << std::setw(30) << plans[i].algorithm
                  << std::setw(10) << RouteUtils::trucksUsed(plans[i])
                  << std::setw(13) << plans[i].unassigned.size()
                  << RouteUtils::totalDistance(problem, plans[i])
                  << (i == best ? "   <- best" : "") << "\n";
    }
    std::cout << std::right;
}
