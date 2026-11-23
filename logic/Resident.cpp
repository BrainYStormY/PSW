// =====================================================================
//  Resident.cpp  -  Behaviour of the Resident agent (US30 - US34)
// ---------------------------------------------------------------------
//  Order of the file follows the agent cycle:
//    1) helper: distance between two cells
//    2) ACT       produceWaste          (US30)
//    3) PERCEIVE  reachableBins         (US31)
//    4) DECIDE    wantsToDispose, chooseBin   (US32)
//    5) DECIDE    reactToFullBin        (US33, US34)
//    6) ACT       disposeWaste          (US31 - US34 together)
//    7) the whole tick: step
// =====================================================================

#include "Agents.hpp"
#include <algorithm>   // std::min, std::clamp
#include <cstddef>     // std::ptrdiff_t
#include <cstdlib>     // std::abs


// ---------------------------------------------------------------------
//  1) Distance between two cells
// ---------------------------------------------------------------------
//  Manhattan distance |dx| + |dy|, the same formula as RoutePlanner::distance
//  (trucks and residents move only horizontally/vertically on a grid without
//  obstacles). It is private in RoutePlanner, so it is repeated here for now.
//  Suggestion: move it to Types.hpp so it exists in exactly one place.
// ---------------------------------------------------------------------
namespace{
    int manhattan(Position a, Position b){
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }
}


// ---------------------------------------------------------------------
//  2) ACT: produce waste (US30)
// ---------------------------------------------------------------------
//  The daily amount is spread evenly over the ticks of a day. The waste is
//  not thrown away immediately: it waits at home in carriedWaste until the
//  resident decides to go to a bin.
// ---------------------------------------------------------------------
void Resident::produceWaste(){
    carriedWaste += wastePerDay / params.ticksPerDay;
}


// ---------------------------------------------------------------------
//  3) PERCEIVE: which bins can I reach? (US31)
// ---------------------------------------------------------------------
//  A bin is reachable if
//    - it is a General bin (recycling is not simulated yet, US81-US85), and
//    - it is at most walkingRange cells away from the resident's house.
//  The list is rebuilt on every trip instead of being stored, so it is always
//  correct even if bins are added, moved or removed (US05-US07).
// ---------------------------------------------------------------------
std::vector<Bin*> Resident::reachableBins(const std::vector<Bin*>& allBins) const{
    std::vector<Bin*> reachable;

    for(Bin* bin : allBins){
        if(bin->getType() != BinType::General) continue;                        // wrong kind of bin
        if(manhattan(position, bin->getPosition()) > params.walkingRange) continue;   // too far away
        reachable.push_back(bin);
    }
    return reachable;
}


// ---------------------------------------------------------------------
//  4a) DECIDE: do I go to a bin in this tick? (US32)
// ---------------------------------------------------------------------
//  Every tick the resident "throws a die". The chance per tick is
//  disposalsPerDay / ticksPerDay, so on average the resident makes
//  disposalsPerDay trips per day, but never at the same time every day.
// ---------------------------------------------------------------------
bool Resident::wantsToDispose(std::mt19937& rng) const{
    if(carriedWaste <= 0.0) return false;       // nothing to throw away

    double chance = std::min(1.0, params.disposalsPerDay / params.ticksPerDay);
    std::bernoulli_distribution goNow(chance);
    return goNow(rng);
}


// ---------------------------------------------------------------------
//  4b) DECIDE: which bin do I try? (US32)
// ---------------------------------------------------------------------
//  Uniformly random among the candidates. The chosen bin is removed from the
//  list, so asking again gives a different bin. Returns nullptr when no
//  candidate is left.
//  Possible refinement: prefer closer bins (weighted choice).
// ---------------------------------------------------------------------
Bin* Resident::chooseBin(std::vector<Bin*>& candidates, std::mt19937& rng) const{
    if(candidates.empty()) return nullptr;

    std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
    std::size_t index = pick(rng);

    Bin* chosen = candidates[index];
    candidates.erase(candidates.begin() + static_cast<std::ptrdiff_t>(index));
    return chosen;
}


// ---------------------------------------------------------------------
//  5) DECIDE: the bin is full, what now? (US33, US34)
// ---------------------------------------------------------------------
//  The resident only finds out that a bin is full when trying to use it
//  (Bin::deposit returns false). Then the awareness decides: with probability
//  "awareness" the resident does not give up and tries another reachable bin.
//  So an eco-conscious resident (0.85) almost always keeps trying, a careless
//  one (0.20) usually goes home and keeps the waste for a later trip.
// ---------------------------------------------------------------------
bool Resident::reactToFullBin(std::mt19937& rng) const{
    std::bernoulli_distribution tryAnother(std::clamp(awareness, 0.0, 1.0));
    return tryAnother(rng);
}


// ---------------------------------------------------------------------
//  6) ACT: one disposal trip (perceive + decide + act)
// ---------------------------------------------------------------------
//  1. perceive: collect the reachable bins                       (US31)
//  2. decide:   pick one of them at random                       (US32)
//  3. act:      try to deposit ALL the waste carried from home
//  4. if the bin was full: react (US33/US34) - try another or give up
//  If no bin accepted the waste, the trip is counted as failed and the waste
//  stays in carriedWaste (it will be thrown away on a later trip).
// ---------------------------------------------------------------------
void Resident::disposeWaste(const std::vector<Bin*>& allBins, std::mt19937& rng){
    trips++;

    std::vector<Bin*> candidates = reachableBins(allBins);      // perceive

    while(Bin* bin = chooseBin(candidates, rng)){                // decide
        if(bin->deposit(carriedWaste, WasteType::General)){      // act: success
            totalDisposed += carriedWaste;
            carriedWaste = 0.0;
            return;
        }
        if(!reactToFullBin(rng)) break;                          // full bin: give up?
    }

    failedTrips++;      // no reachable bin, or every bin tried was full / the resident gave up
}


// ---------------------------------------------------------------------
//  7) One tick of the resident
// ---------------------------------------------------------------------
//  Called once per tick by Simulation::step() (replaces the plain
//  produceWaste() call there).
// ---------------------------------------------------------------------
void Resident::step(const std::vector<Bin*>& allBins, std::mt19937& rng){
    produceWaste();                         // waste accumulates at home (US30)

    if(wantsToDispose(rng)){                // random moment for a trip (US32)
        disposeWaste(allBins, rng);
    }
}
