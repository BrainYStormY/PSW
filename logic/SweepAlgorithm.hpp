#pragma once

#include "RoutingAlgorithm.hpp"


// Sweep: sorts the bins by angle around the depot (like a clock hand)
// and cuts them into slices, one slice per truck, until each truck is full.
// Then each truck's order comes from RoutePlanner (nearest neighbour + 2-opt).
class SweepAlgorithm : public RoutingAlgorithm{
public:
    std::string name() const override {return "Sweep";}
    Plan solve(const Problem& problem) const override;
};
