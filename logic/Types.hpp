#pragma once


enum class BinType{
    General,
    Recycling
};
enum class WasteType{
    General,
    Recyclable
};
enum class TruckState{
    Idle,
    Moving,
    Collecting,
    Unloading,
    BrokenDown
};

struct Position{
    int x;
    int y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};
