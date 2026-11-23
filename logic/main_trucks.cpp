// =====================================================================
//  main.cpp  -  WasteFlow: routes for several trucks
// ---------------------------------------------------------------------
//  Reads depot, trucks and bins from a text file (see InputReader.hpp),
//  lets the user choose an algorithm and shows the trajectories.
//
//  Build:  make            (or see the Makefile for the g++ line)
//  Run:    ./wasteflow data/city1.txt
// =====================================================================

#include "Types.hpp"
#include "Environment.hpp"
#include "RoutePlanner.hpp"
#include "Problem.hpp"
#include "InputReader.hpp"
#include "Display.hpp"
#include "Improvement.hpp"
#include "SweepAlgorithm.hpp"
#include "KMeansAlgorithm.hpp"
#include "ClarkeWrightAlgorithm.hpp"
#include "InsertionAlgorithm.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <cstdlib>   // std::exit


// ---------------------------------------------------------------------
//  KEYBOARD INPUT
// ---------------------------------------------------------------------
//  Always read the whole line with getline and only then look at it,
//  so every Enter gets an answer (cin >> can wait forever for input).
// ---------------------------------------------------------------------

// Reads a whole number. Returns -1 if the line is not a valid number.
int readInt(){
    std::string line;
    if(!std::getline(std::cin, line)) std::exit(0);   // end of input (ex: Ctrl+D) -> quit
    std::istringstream in(line);
    int n;
    if(!(in >> n) || n < 0) return -1;
    return n;
}

// Asks a yes/no question until the answer starts with y or n.
bool askYesNo(const std::string& question){
    while(true){
        std::cout << question;
        std::string line;
        if(!std::getline(std::cin, line)) std::exit(0);
        if(!line.empty() && (line[0] == 'y' || line[0] == 'Y')) return true;
        if(!line.empty() && (line[0] == 'n' || line[0] == 'N')) return false;
    }
}


// Runs one algorithm and, if asked, the improvement.
// Prints the distance before and after so the effect is visible.
Plan run(const RoutingAlgorithm& algorithm, const Problem& problem, bool improve, bool verbose){
    Plan plan = algorithm.solve(problem);
    if(improve){
        int before = RouteUtils::totalDistance(problem, plan);
        Improvement::apply(problem, plan);
        plan.algorithm += " + improvement";
        if(verbose){
            std::cout << "\nDistance before improvement: " << before
                      << " | after: " << RouteUtils::totalDistance(problem, plan) << "\n";
        }
    }
    return plan;
}


// Shows a plan: summary, map with every truck and, if wanted, details per truck
void show(const Problem& problem, const Plan& plan){
    Display::printPlan(problem, plan);
    Display::printOverviewMap(problem, plan);
    if(askYesNo("\nShow step by step and arrow map for each truck? (y/n): ")){
        Display::printTruckDetails(problem, plan);
    }
}


int main(int argc, char* argv[]){
    // 1) Input file: from the command line or typed by the user
    std::string path;
    if(argc >= 2){
        path = argv[1];
    }
    else{
        std::cout << "Input file (ex: data/city1.txt): ";
        if(!std::getline(std::cin, path)) return 1;
    }

    Problem problem;
    std::string error;
    if(!InputReader::read(path, problem, error)){
        std::cerr << "Error reading '" << path << "': " << error << "\n";
        return 1;
    }
    Display::printProblem(problem);

    // 2) Available algorithms (to add one: write the class and push it here)
    std::vector<std::unique_ptr<RoutingAlgorithm>> algorithms;
    algorithms.push_back(std::make_unique<SweepAlgorithm>());
    algorithms.push_back(std::make_unique<KMeansAlgorithm>());
    algorithms.push_back(std::make_unique<ClarkeWrightAlgorithm>());
    algorithms.push_back(std::make_unique<InsertionAlgorithm>());

    // 3) Menu
    while(true){
        std::cout << "\nChoose an algorithm:\n";
        for(size_t i = 0; i < algorithms.size(); i++){
            std::cout << "  " << i+1 << ") " << algorithms[i]->name() << "\n";
        }
        int runAll = algorithms.size() + 1;
        std::cout << "  " << runAll << ") Run all and pick the cheapest\n";
        std::cout << "  0) Exit\n> ";

        int choice = readInt();
        if(choice == 0) break;
        if(choice < 1 || choice > runAll){
            std::cout << "Invalid option.\n";
            continue;
        }

        bool improve = askYesNo("Apply improvement (2-opt + relocate)? (y/n): ");

        if(choice != runAll){
            Plan plan = run(*algorithms[choice-1], problem, improve, true);
            show(problem, plan);
        }
        else{
            // run every algorithm and keep the best plan
            std::vector<Plan> plans;
            size_t best = 0;
            for(const auto& algorithm : algorithms){
                plans.push_back(run(*algorithm, problem, improve, false));
                if(RouteUtils::isBetter(problem, plans.back(), plans[best])) best = plans.size() - 1;
            }
            Display::printComparison(problem, plans, best);
            show(problem, plans[best]);
        }
    }
    return 0;   // the Problem deletes the bins by itself
}
