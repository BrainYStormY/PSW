#pragma once

#include "Problem.hpp"
#include <string>


// Base class for every multi-truck routing algorithm (Strategy pattern).
// Each algorithm receives the same Problem and returns a Plan,
// so main can run any of them, or all of them, the same way.
class RoutingAlgorithm{
public:
    virtual ~RoutingAlgorithm() = default;

    virtual std::string name() const = 0;
    virtual Plan solve(const Problem& problem) const = 0;
};
