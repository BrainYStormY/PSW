#pragma once

#include "Problem.hpp"
#include <vector>


// Everything that is printed on the screen.
// Trucks are shown with letters: A, B, C... (in the order of the input file).
namespace Display{
    // What was read from the file
    void printProblem(const Problem& problem);

    // One line per truck: stops, load and distance
    void printPlan(const Problem& problem, const Plan& plan);

    // One map with every truck (letters)
    void printOverviewMap(const Problem& problem, const Plan& plan);

    // For each truck: step by step + map with arrows
    void printTruckDetails(const Problem& problem, const Plan& plan);

    // Table with the result of several algorithms
    void printComparison(const Problem& problem, const std::vector<Plan>& plans, size_t best);
}
