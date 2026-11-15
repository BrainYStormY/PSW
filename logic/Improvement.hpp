#pragma once

#include "Problem.hpp"


// Improves any plan (from any algorithm). Repeats until nothing gets better:
//   1) 2-opt inside each truck's route (RoutePlanner::twoOpt);
//   2) relocate: move one bin to another truck if the total distance goes down.
class Improvement{
public:
    static void apply(const Problem& problem, Plan& plan);
};
