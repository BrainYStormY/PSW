#pragma once

#include "RoutingAlgorithm.hpp"


// Cheapest insertion: starts with every truck empty and, at each step,
// puts the bin that adds the least distance in the best place of the best truck.
// Builds groups AND order at the same time.
class InsertionAlgorithm : public RoutingAlgorithm{
public:
    std::string name() const override {return "Insertion";}
    Plan solve(const Problem& problem) const override;
};
