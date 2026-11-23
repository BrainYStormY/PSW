#pragma once

// =====================================================================
//  PopulationConfig.hpp  -  Settings that describe the resident population
//  (US26, US28, US29, US30, US34)
// ---------------------------------------------------------------------
//  Plain data only, no logic. The GUI / Python side will fill these values
//  later (US44), so NOTHING in here is hard-coded: every number comes from
//  whoever creates the PopulationConfig.
//
//  Units used on the C++ side:
//    waste      -> kg
//    distances  -> grid cells (Manhattan distance, same as the RoutePlanner)
//    time       -> ticks
//  Conversions from the GUI units (metres, minutes) must happen in ONE
//  place, when the config is built (see open question "Units" in
//  PROJECT_CONTEXT).
// =====================================================================

#include <string>
#include <vector>


// ---------------------------------------------------------------------
//  One TYPE of resident (US28). Values match the profile table of the
//  GUI mockup: name, share %, kg/day, awareness.
// ---------------------------------------------------------------------
struct ResidentType{
    std::string name;       // e.g. "Eco-Conscious"
    double sharePercent;    // US29: percentage of the population, 0..100
    double wastePerDay;     // US30: kg of waste produced per day
    double awareness;       // US34: environmental awareness, 0.0 .. 1.0
};


// ---------------------------------------------------------------------
//  Parameters that are the same for every resident.
// ---------------------------------------------------------------------
struct ResidentParams{
    int ticksPerDay;        // how many ticks make one simulated day
                            // (Python side: 1440 / tick_duration)
    int walkingRange;       // US31: farthest bin a resident will walk to, in cells
    double disposalsPerDay; // US32: average number of disposal trips per day
};


// ---------------------------------------------------------------------
//  Everything needed to create the whole population.
// ---------------------------------------------------------------------
struct PopulationConfig{
    int totalResidents;                 // US26
    std::vector<ResidentType> types;    // US28 (shares must add up to 100, US29)
    ResidentParams params;
};
