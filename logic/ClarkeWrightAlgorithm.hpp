#pragma once

#include "RoutingAlgorithm.hpp"


// Clarke-Wright (savings): starts with one route per bin (depot -> bin -> depot)
// and keeps joining the two routes that save the most distance,
// as long as the result fits in a truck. Builds groups AND order at the same time.
class ClarkeWrightAlgorithm : public RoutingAlgorithm{
public:
    std::string name() const override {return "Clarke-Wright";}
    Plan solve(const Problem& problem) const override;
};
