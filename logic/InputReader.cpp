// =====================================================================
//  InputReader.cpp  -  Reads depot, trucks and bins from a text file
// =====================================================================

#include "InputReader.hpp"
#include <fstream>
#include <sstream>
#include <set>
#include <cctype>   // std::toupper


bool InputReader::read(const std::string& path, Problem& problem, std::string& error){
    std::ifstream file(path);
    if(!file){
        error = "could not open file '" + path + "'";
        return false;
    }

    std::string line;
    int lineNumber = 0;
    bool hasDepot = false;

    while(std::getline(file, line)){
        lineNumber++;
        std::string where = "line " + std::to_string(lineNumber) + ": ";

        // drop everything after '#'
        size_t hash = line.find('#');
        if(hash != std::string::npos) line = line.substr(0, hash);

        std::istringstream in(line);
        std::string key;
        if(!(in >> key)) continue;                     // empty line
        for(char& c : key) c = std::toupper(c);        // "bin" and "BIN" both work

        if(key == "GRID"){
            int size;
            if(!(in >> size) || size <= 0){ error = where + "expected 'GRID size'"; return false; }
            problem.gridSize = size;
        }
        else if(key == "DEPOT"){
            Position p;
            if(!(in >> p.x >> p.y)){ error = where + "expected 'DEPOT x y'"; return false; }
            problem.depot = p;
            hasDepot = true;
        }
        else if(key == "TRUCK"){
            TruckInfo t;
            if(!(in >> t.id >> t.start.x >> t.start.y >> t.capacity) || t.capacity <= 0){
                error = where + "expected 'TRUCK id x y capacity' (capacity > 0)";
                return false;
            }
            problem.trucks.push_back(t);
        }
        else if(key == "BIN"){
            int id;
            Position p;
            double load;
            if(!(in >> id >> p.x >> p.y >> load) || load < 0){
                error = where + "expected 'BIN id x y load' (load >= 0)";
                return false;
            }
            // uses the Bin class from Environment.hpp; type and capacity do not matter for routing yet
            Bin* bin = new Bin(id, p, BinType::General, 100.0);
            bin->deposit(load, WasteType::General);
            problem.bins.push_back(bin);
        }
        else{
            error = where + "unknown keyword '" + key + "'";
            return false;
        }
    }

    // ---- checks after reading everything ----
    if(!hasDepot){ error = "missing DEPOT line"; return false; }
    if(problem.trucks.empty()){ error = "no TRUCK lines"; return false; }

    auto inside = [&](Position p){
        return p.x >= 0 && p.x < problem.gridSize && p.y >= 0 && p.y < problem.gridSize;
    };
    std::string limits = " is outside the grid (0.." + std::to_string(problem.gridSize - 1) + ")";

    if(!inside(problem.depot)){ error = "depot" + limits; return false; }

    std::set<int> truckIds, binIds;
    for(const TruckInfo& t : problem.trucks){
        if(!inside(t.start)){ error = "truck " + std::to_string(t.id) + limits; return false; }
        if(!truckIds.insert(t.id).second){ error = "repeated truck id " + std::to_string(t.id); return false; }
    }
    for(Bin* b : problem.bins){
        if(!inside(b->getPosition())){ error = "bin " + std::to_string(b->getId()) + limits; return false; }
        if(!binIds.insert(b->getId()).second){ error = "repeated bin id " + std::to_string(b->getId()); return false; }
    }
    return true;
}
