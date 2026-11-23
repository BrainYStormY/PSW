#pragma once

#include "Types.hpp"
#include <vector>
#include <memory>


class Truck;
class Resident;


class Building{
protected:
    int id;
    Position position;

public:
// Constructor
    Building(int id, Position pos) : id(id), position(pos){};
// Destructor
    virtual ~Building() = default;
// Getters
    int getId() const {return id;}
    Position getPosition() const {return position;}

};


class House : public Building{
private:
    std::vector<Resident*> residents;

public:
// Constructor
    House(int id, Position pos) : Building(id, pos){};
// Getters
    const std::vector<Resident*>& getResidents() const {return residents;}   // needed to check US27
// Setters
    void addResident(Resident* res){residents.push_back(res);}
};


class Depot : public Building{
private:
    std::vector<Truck*> parkedTrucks;

public:
// Constructor
    Depot(int id, Position pos) : Building(id, pos){};
// Getters
    bool hasTruck(Truck* truck);
// Setters
    void parkTruck(Truck* truck);
    void releaseTruck(Truck* truck);
};


class Landfill : public Building{
private:
    double totalReceived = 0.0;

public:
// Constructor
    Landfill(int id, Position pos) : Building(id, pos){};
// Getters
    double getTotalReceived() const {return totalReceived;}
// Setters
    void receive(double amount){totalReceived += amount;}
};


class RecyclingCenter : public Building{
private:
    double capacity;
    double currentLoad = 0.0;

public:
// Constructor
    RecyclingCenter(int id, Position pos, double cap) : Building(id, pos), capacity(cap){};
};


class Bin{
private:
    int id;
    Position position;
    BinType type;
    double capacity;
    double fillLevel = 0.0;
    double wronglySorted = 0.0;

public:
// Constructor
    Bin(int id, Position pos, BinType type, double cap) : id(id), position(pos), type(type), capacity(cap){};
// Getters
    // getId/getPosition: main.cpp and PROJECT_CONTEXT say Bin already has them,
    // but they are missing in this copy of the file -> merge with Bruno's version.
    int getId() const {return id;}
    Position getPosition() const {return position;}
    BinType getType() const {return type;}          // residents only use General bins (US31)
    double getCapacity() const {return capacity;}
    double getFillLevel() const {return fillLevel;}
    bool isFull() const {return fillLevel >= capacity;}   // "const" added
// Setters
    bool deposit(double amount, WasteType wasteType){
        if(isFull())return false;
        fillLevel += amount;

        if((type == BinType::General && wasteType == WasteType::Recyclable) || (type == BinType::Recycling && wasteType == WasteType::General)){
            wronglySorted += amount;
        }

        return true;
    }

    double empty(){
        double collected = fillLevel;
        fillLevel = 0.0;
        wronglySorted = 0.0;
        return collected;
    }
};
