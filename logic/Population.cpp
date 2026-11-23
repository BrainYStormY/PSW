// =====================================================================
//  Population.cpp  -  Creates the residents of the city (US26 - US29)
// ---------------------------------------------------------------------
//  Order of the file:
//    1) validatePopulationConfig   (US26, US29, US30, US34)
//    2) countResidentsPerType      (US28, US29)
//    3) buildPopulation            (US26, US27)
// =====================================================================

#include "Population.hpp"
#include <algorithm>   // std::shuffle, std::stable_sort
#include <cmath>       // std::floor, std::fabs
#include <cstddef>     // std::size_t
#include <numeric>     // std::iota
#include <stdexcept>   // std::invalid_argument
#include <string>


namespace{
    // The shares are doubles typed by a person (e.g. 33.3 + 33.3 + 33.4),
    // so "adds up to 100" is checked with a small tolerance.
    const double SHARE_TOLERANCE = 1e-6;
}


// ---------------------------------------------------------------------
//  1) Validation
// ---------------------------------------------------------------------
//  One single place that knows which values are allowed. The GUI can show
//  the message of the exception to the technician.
// ---------------------------------------------------------------------
void validatePopulationConfig(const PopulationConfig& config){
    // US26: total number of residents
    if(config.totalResidents < 0){
        throw std::invalid_argument("totalResidents must not be negative");
    }
    if(config.types.empty()){
        throw std::invalid_argument("at least one resident type is required");
    }

    // US28, US29, US30, US34: check every resident type and add up the shares
    double shareSum = 0.0;
    for(const ResidentType& type : config.types){
        if(type.sharePercent < 0.0 || type.sharePercent > 100.0){
            throw std::invalid_argument(type.name + ": sharePercent must be between 0 and 100");
        }
        if(type.wastePerDay < 0.0){
            throw std::invalid_argument(type.name + ": wastePerDay must not be negative");
        }
        if(type.awareness < 0.0 || type.awareness > 1.0){
            throw std::invalid_argument(type.name + ": awareness must be between 0 and 1");
        }
        shareSum += type.sharePercent;
    }

    // US29: the percentages of all types must make 100 %
    if(std::fabs(shareSum - 100.0) > SHARE_TOLERANCE){
        throw std::invalid_argument("the shares of all types must add up to 100 %, but they add up to "
                                    + std::to_string(shareSum));
    }

    // Behaviour parameters shared by all residents
    if(config.params.ticksPerDay < 1){
        throw std::invalid_argument("ticksPerDay must be at least 1");
    }
    if(config.params.walkingRange < 0){
        throw std::invalid_argument("walkingRange must not be negative");
    }
    if(config.params.disposalsPerDay < 0.0){
        throw std::invalid_argument("disposalsPerDay must not be negative");
    }
}


// ---------------------------------------------------------------------
//  2) How many residents of each type? (US28, US29)
// ---------------------------------------------------------------------
//  Example: 10 residents with shares 33.3 / 33.3 / 33.4 give the exact
//  values 3.33 / 3.33 / 3.34. Rounding each one would give 3+3+3 = 9.
//  Largest remainder method:
//    a) every type first gets the whole part of its exact value (3, 3, 3)
//    b) the residents still missing (1) go to the types with the biggest
//       fractional part (0.34 -> third type), one each.
//  The result always adds up to totalResidents.
// ---------------------------------------------------------------------
std::vector<int> countResidentsPerType(const PopulationConfig& config){
    validatePopulationConfig(config);

    const std::size_t typeCount = config.types.size();
    std::vector<int> counts(typeCount, 0);
    std::vector<double> remainders(typeCount, 0.0);
    int assigned = 0;

    // a) whole part of each type
    for(std::size_t i = 0; i < typeCount; i++){
        double exact = config.totalResidents * config.types[i].sharePercent / 100.0;
        counts[i] = static_cast<int>(std::floor(exact));
        remainders[i] = exact - counts[i];
        assigned += counts[i];
    }

    // b) hand out the missing residents, biggest remainder first
    std::vector<std::size_t> order(typeCount);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&remainders](std::size_t a, std::size_t b){return remainders[a] > remainders[b];});

    for(std::size_t k = 0; assigned < config.totalResidents; k++){
        counts[order[k % typeCount]]++;
        assigned++;
    }
    return counts;
}


// ---------------------------------------------------------------------
//  3) Create the residents and link them to houses (US26, US27)
// ---------------------------------------------------------------------
//  Steps:
//    a) check the config and the houses
//    b) make a list with one entry per resident saying which type it has
//       (e.g. 60 x Eco, 100 x Standard, 40 x Careless) and shuffle it, so the
//       types are mixed across the city instead of one type per street
//    c) create resident i, put it in house (i modulo number of houses): the
//       residents are spread evenly over the houses (difference of at most 1)
//    d) the resident starts at the position of its house, which is its fixed
//       place in the city (US27); the house gets a pointer back to the resident
//  The shuffle uses the simulation's seeded engine, so the same seed always
//  gives the same city (US42).
// ---------------------------------------------------------------------
std::vector<Resident*> buildPopulation(const PopulationConfig& config,
                                       const std::vector<House*>& houses,
                                       std::mt19937& rng){
    // a) checks
    validatePopulationConfig(config);
    if(config.totalResidents > 0 && houses.empty()){
        throw std::invalid_argument("residents need at least one house to live in (US27)");
    }

    // b) one type index per resident, then mix them
    std::vector<int> counts = countResidentsPerType(config);
    std::vector<std::size_t> typeOfResident;
    typeOfResident.reserve(static_cast<std::size_t>(config.totalResidents));
    for(std::size_t t = 0; t < counts.size(); t++){
        typeOfResident.insert(typeOfResident.end(), static_cast<std::size_t>(counts[t]), t);
    }
    std::shuffle(typeOfResident.begin(), typeOfResident.end(), rng);

    // c) + d) create the residents
    std::vector<Resident*> residents;
    residents.reserve(typeOfResident.size());
    for(std::size_t i = 0; i < typeOfResident.size(); i++){
        const ResidentType& type = config.types[typeOfResident[i]];
        House* house = houses[i % houses.size()];

        Resident* resident = new Resident(static_cast<int>(i) + 1,      // ids start at 1, like the bins
                                          house->getPosition(),         // US27: lives at the house
                                          type.wastePerDay,             // US30
                                          type.awareness,               // US34
                                          house,
                                          type.name,                    // US28
                                          config.params);
        house->addResident(resident);
        residents.push_back(resident);
    }
    return residents;
}
