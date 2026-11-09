// =====================================================================
//  RoutePlanner.cpp  -  Implementacao do planeador de rotas
//  (ver explicacao geral do metodo em RoutePlanner.hpp)
// =====================================================================

#include "RoutePlanner.hpp"
#include <cstdlib>     // std::abs
#include <algorithm>   // std::reverse
#include <climits>     // INT_MAX


// Distancia de Manhattan entre duas celulas: quantas celulas o camiao
// tem de andar na horizontal mais quantas tem de andar na vertical.
// Ex: (0,0) -> (3,4) = 3 + 4 = 7 celulas.
int RoutePlanner::distance(Position a, Position b){
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}


// Soma as distancias de todos os trocos da rota, incluindo a ida
// do deposito ao 1.o caixote e o regresso do ultimo ao deposito.
int RoutePlanner::routeLength(const std::vector<Bin*>& order) const{
    if(order.empty()) return 0;

    int total = distance(depot, order.front()->getPosition());           // deposito -> 1.o
    for(size_t i = 1; i < order.size(); i++){
        total += distance(order[i-1]->getPosition(), order[i]->getPosition());  // caixote -> caixote
    }
    total += distance(order.back()->getPosition(), depot);                // ultimo -> deposito
    return total;
}


// ---------------------------------------------------------------------
//  PASSO 1 - Vizinho mais proximo
// ---------------------------------------------------------------------
//  Comeca no deposito. Em cada passo procura, entre os caixotes ainda
//  nao visitados, o que esta mais perto da posicao atual, vai para la
//  e marca-o como visitado. Repete ate visitar todos.
//  E rapido e simples, mas por ser "guloso" (so olha para o passo
//  seguinte) pode deixar a rota com cruzamentos e voltas desnecessarias,
//  sobretudo no regresso ao deposito. Isso e corrigido no passo 2.
// ---------------------------------------------------------------------
std::vector<Bin*> RoutePlanner::nearestNeighbour(const std::vector<Bin*>& bins) const{
    std::vector<Bin*> order;
    std::vector<bool> visited(bins.size(), false);   // visited[i] -> o caixote i ja foi escolhido?
    Position current = depot;

    for(size_t step = 0; step < bins.size(); step++){
        int best = -1;           // indice do caixote mais proximo encontrado
        int bestDist = INT_MAX;  // comeca com "infinito" para qualquer distancia ser menor

        for(size_t i = 0; i < bins.size(); i++){
            if(visited[i]) continue;
            int d = distance(current, bins[i]->getPosition());
            if(d < bestDist){
                bestDist = d;
                best = i;
            }
        }

        // vai para o mais proximo
        visited[best] = true;
        order.push_back(bins[best]);
        current = bins[best]->getPosition();
    }
    return order;
}


// ---------------------------------------------------------------------
//  PASSO 2 - Melhoria 2-opt
// ---------------------------------------------------------------------
//  Ideia: escolher um troco da rota (do caixote i ao caixote j) e
//  percorre-lo ao contrario. Exemplo com a ordem A B C D E:
//      inverter de B a D  ->  A D C B E
//  Isto troca duas ligacoes da rota (A-B e D-E passam a A-D e B-E).
//  Quando a rota original tinha um "cruzamento", esta inversao desfaz-o
//  e a rota fica mais curta.
//
//  Testam-se todos os pares (i, j):
//    - se a rota ficar mais curta, a inversao fica;
//    - se nao, volta-se a inverter para desfazer.
//  Repete-se tudo enquanto houver alguma melhoria. Quando uma passagem
//  completa nao melhorar nada, a rota ja nao se pode melhorar com 2-opt.
//
//  Nota: o deposito nao esta no vetor, por isso nunca sai do inicio
//  nem do fim; routeLength ja o inclui nas contas.
// ---------------------------------------------------------------------
void RoutePlanner::twoOpt(std::vector<Bin*>& order) const{
    bool improved = true;

    while(improved){
        improved = false;
        int bestLength = routeLength(order);

        for(size_t i = 0; i + 1 < order.size(); i++){
            for(size_t j = i + 1; j < order.size(); j++){
                // inverte o troco entre i e j (inclusive)
                std::reverse(order.begin() + i, order.begin() + j + 1);
                int newLength = routeLength(order);

                if(newLength < bestLength){
                    bestLength = newLength;   // melhorou: fica assim
                    improved = true;
                }
                else{
                    std::reverse(order.begin() + i, order.begin() + j + 1);   // nao melhorou: desfaz
                }
            }
        }
    }
}


// Junta os dois passos: rota inicial + melhoria
std::vector<Bin*> RoutePlanner::planOrder(const std::vector<Bin*>& bins) const{
    std::vector<Bin*> order = nearestNeighbour(bins);
    twoOpt(order);
    return order;
}


// ---------------------------------------------------------------------
//  Trajetoria celula a celula
// ---------------------------------------------------------------------
//  A ordem diz QUAIS caixotes visitar e por que ordem; aqui calcula-se
//  POR ONDE o camiao passa. Entre cada par de paragens anda primeiro
//  tudo o que tem a andar em x e depois tudo em y (forma de "L").
//  Qualquer caminho so com passos horizontais/verticais na direcao
//  certa tem o mesmo comprimento (Manhattan), por isso esta escolha
//  nao altera a distancia total.
// ---------------------------------------------------------------------
std::vector<Position> RoutePlanner::buildPath(const std::vector<Bin*>& order) const{
    std::vector<Position> path;
    Position current = depot;
    path.push_back(current);

    // destinos por ordem: os caixotes e, no fim, o regresso ao deposito
    std::vector<Position> targets;
    for(Bin* bin : order) targets.push_back(bin->getPosition());
    targets.push_back(depot);

    for(Position target : targets){
        // anda uma celula de cada vez em x ate acertar a coluna...
        while(current.x != target.x){
            current.x += (target.x > current.x) ? 1 : -1;
            path.push_back(current);
        }
        // ...e depois em y ate acertar a linha
        while(current.y != target.y){
            current.y += (target.y > current.y) ? 1 : -1;
            path.push_back(current);
        }
    }
    return path;
}