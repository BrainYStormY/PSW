#pragma once

// =====================================================================
//  Population.hpp  -  Creates the residents of the city (US26 - US29)
// =====================================================================

#include "Agents.hpp"
#include "PopulationConfig.hpp"
#include <random>
#include <vector>


// Checks that a PopulationConfig makes sense (US26, US29, US30, US34).
// Throws std::invalid_argument with a readable message if it does not.
void validatePopulationConfig(const PopulationConfig& config);

// How many residents of each type there are (US28, US29).
// counts[i] belongs to config.types[i]; the counts always add up to
// config.totalResidents (largest remainder method, no resident is lost to rounding).
std::vector<int> countResidentsPerType(const PopulationConfig& config);

// Creates all residents and links each one to a house (US26, US27).
//  - The caller OWNS the returned residents (delete them, or hand them over to
//    the Simulation). Houses only keep non-owning pointers.
//  - rng must be the simulation's seeded engine, so the result is reproducible.
//  - Throws std::invalid_argument if the config is invalid, or if there are
//    residents to create but no house.
std::vector<Resident*> buildPopulation(const PopulationConfig& config,
                                       const std::vector<House*>& houses,
                                       std::mt19937& rng);
