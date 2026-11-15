#pragma once

#include "Types.hpp"
#include "Environment.hpp"
#include <vector>


class RoutePlanner{
private:
    Position depot;

    // Distancia numa grelha sem obstaculos (o camiao so anda na horizontal/vertical)
    static int distance(Position a, Position b);

    // Comprimento total: depot -> caixotes pela ordem dada -> depot
    int routeLength(const std::vector<Bin*>& order) const;

    // Passo 1: vizinho mais proximo
    std::vector<Bin*> nearestNeighbour(const std::vector<Bin*>& bins) const;

    // Passo 2: melhorar com 2-opt
    void twoOpt(std::vector<Bin*>& order) const;

public:
// Constructor
    RoutePlanner(Position depotPos) : depot(depotPos){};

// Devolve a ordem pela qual o camiao deve visitar os caixotes
    std::vector<Bin*> planOrder(const std::vector<Bin*>& bins) const;

// Devolve todas as celulas da grelha por onde o camiao passa (trajetoria completa)
    std::vector<Position> buildPath(const std::vector<Bin*>& order) const;

    int totalDistance(const std::vector<Bin*>& order) const {return routeLength(order);}
};
