#pragma once

#include "Problem.hpp"
#include <string>


// Reads the city from a text file.
//
// File format (one item per line, '#' starts a comment):
//   GRID  size                  (optional, default 30)
//   DEPOT x y
//   TRUCK id x y capacity
//   BIN   id x y load
//
// Coordinates go from 0 to size-1. Load and capacity use the same unit (e.g. kg).
class InputReader{
public:
    // Fills 'problem'. Returns false and writes a message in 'error' if something is wrong.
    static bool read(const std::string& path, Problem& problem, std::string& error);
};
