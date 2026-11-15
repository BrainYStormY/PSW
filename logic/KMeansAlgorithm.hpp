#pragma once

#include "RoutingAlgorithm.hpp"


// K-means: groups the bins by proximity into k groups (k = number of trucks),
// without going over each truck's capacity.
// Then each truck's order comes from RoutePlanner (nearest neighbour + 2-opt).
class KMeansAlgorithm : public RoutingAlgorithm{
public:
    std::string name() const override {return "K-means";}
    Plan solve(const Problem& problem) const override;
};
