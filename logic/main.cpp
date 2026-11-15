// =====================================================================
//  main.cpp  -  Teste do planeador de rotas do WasteFlow
// ---------------------------------------------------------------------
//  Versao simplificada: 1 camiao, 1 deposito e N caixotes numa grelha
//  30x30. O utilizador indica as posicoes pelo teclado e o programa:
//    1) calcula a ordem pela qual o camiao deve visitar os caixotes
//       (RoutePlanner::planOrder -> vizinho mais proximo + 2-opt);
//    2) gera a trajetoria celula a celula (RoutePlanner::buildPath);
//    3) mostra o resultado em texto (troco a troco) e num mapa com setas.
//
//  Compilar:  g++ -std=c++17 -Wall main.cpp RoutePlanner.cpp -o wasteflow
//  Correr:    ./wasteflow
//
//  Nota: nao se inclui o SimulationEngine.hpp porque, por agora, nao e
//  preciso (e o Statistics::update e privado mas e chamado pelo
//  Simulation, o que vai dar erro quando for implementado).
// =====================================================================

#include "Types.hpp"
#include "Environment.hpp"
#include "RoutePlanner.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>   // std::istringstream, para interpretar cada linha lida
#include <cstdlib>   // std::exit, std::abs

// Tamanho da grelha (GRID x GRID). Coordenadas validas: 0 .. GRID-1
const int GRID = 30;


// ---------------------------------------------------------------------
//  LEITURA DO TECLADO
// ---------------------------------------------------------------------
//  Primeira versao usava "std::cin >> p.x >> p.y". Problema: se o
//  utilizador escrevia so um numero (ou usava virgula), o cin ficava
//  a espera do segundo numero e parecia que o programa tinha parado.
//  Solucao: ler SEMPRE a linha inteira com getline e so depois
//  interpretar o que la esta. Assim cada Enter tem sempre resposta.
// ---------------------------------------------------------------------

// Le uma posicao. Aceita "5 7", "5,7" ou "(5,7)".
// Devolve false se a linha nao tiver dois numeros ou se sairem da grelha.
bool readPosition(Position& p){
    std::string line;
    if(!std::getline(std::cin, line)) std::exit(1);   // fim do input (ex: Ctrl+D) -> sai

    // troca virgulas e parenteses por espacos, para "5,7" e "(5,7)" virarem "5 7"
    for(char& c : line){
        if(c == ',' || c == '(' || c == ')') c = ' ';
    }

    // le os dois numeros a partir da linha (como se fosse um cin so dessa linha)
    std::istringstream in(line);
    if(!(in >> p.x >> p.y)) return false;

    // confirma que a posicao esta dentro da grelha
    return p.x >= 0 && p.x < GRID && p.y >= 0 && p.y < GRID;
}

// Le um inteiro nao negativo (numero de caixotes). Devolve -1 se for invalido.
int readInt(){
    std::string line;
    if(!std::getline(std::cin, line)) std::exit(1);
    std::istringstream in(line);
    int n;
    if(!(in >> n) || n < 0) return -1;
    return n;
}


// ---------------------------------------------------------------------
//  MAPA COM SETAS
// ---------------------------------------------------------------------
//  Constroi uma matriz de caracteres GRID x GRID e desenha:
//    '.'          celula por onde o camiao nao passa
//    > < ^ v      sentido em que o camiao SAI dessa celula
//    1, 2, 3...   caixotes, numerados pela ORDEM de recolha
//                 (nao pelo id: o C4 pode aparecer como "2")
//    'B'          caixote a partir do 10.o (so ha um digito por celula)
//    'D'          deposito
//  A ordem de desenho importa: primeiro as setas, depois os caixotes e
//  por fim o deposito, para estes nao serem tapados pelas setas.
//
//  Atencao: no ecra o y CRESCE PARA BAIXO (linha 0 e a de cima),
//  por isso andar para y+1 desenha-se com 'v'.
// ---------------------------------------------------------------------
void printGrid(const Position& depot, const std::vector<Bin*>& order, const std::vector<Position>& path){
    // grid[y][x] -> primeiro indice e a linha (y), segundo e a coluna (x)
    std::vector<std::vector<char>> grid(GRID, std::vector<char>(GRID, '.'));

    // compara cada celula do caminho com a seguinte para saber a direcao
    for(size_t i = 0; i + 1 < path.size(); i++){
        Position a = path[i], b = path[i+1];
        char arrow;
        if(b.x > a.x)      arrow = '>';
        else if(b.x < a.x) arrow = '<';
        else if(b.y > a.y) arrow = 'v';    // y cresce para baixo no ecra
        else               arrow = '^';
        grid[a.y][a.x] = arrow;            // se o camiao passar duas vezes, fica a ultima seta
    }

    // caixotes: '1' + i da o caracter '1', '2', ... ate '9'
    for(size_t i = 0; i < order.size(); i++){
        Position p = order[i]->getPosition();
        grid[p.y][p.x] = (i < 9) ? char('1' + i) : 'B';
    }
    grid[depot.y][depot.x] = 'D';

    // cabecalho com o x (so o algarismo das unidades, para as colunas alinharem)
    std::cout << "    ";
    for(int x = 0; x < GRID; x++) std::cout << x % 10 << ' ';
    std::cout << "  <- x\n";

    // cada linha comeca com o seu y (com espaco extra se tiver 1 algarismo)
    for(int y = 0; y < GRID; y++){
        std::cout << (y < 10 ? " " : "") << y << "  ";
        for(int x = 0; x < GRID; x++) std::cout << grid[y][x] << ' ';
        std::cout << '\n';
    }
}


// ---------------------------------------------------------------------
//  DESCRICAO EM TEXTO
// ---------------------------------------------------------------------
//  Para cada troco (deposito -> 1.o caixote -> ... -> deposito) mostra
//  quantas celulas o camiao anda para cada lado e o total do troco.
//  dx = destino.x - atual.x  (positivo = direita, negativo = esquerda)
//  dy = destino.y - atual.y  (positivo = baixo,   negativo = cima)
//  A soma |dx| + |dy| e a distancia de Manhattan desse troco.
// ---------------------------------------------------------------------
void printSteps(const Position& depot, const std::vector<Bin*>& order){
    // lista de paragens pela ordem calculada, terminando no deposito
    std::vector<Position> stops;
    std::vector<std::string> names;
    for(Bin* b : order){
        stops.push_back(b->getPosition());
        names.push_back("C" + std::to_string(b->getId()));
    }
    stops.push_back(depot);
    names.push_back("D");

    Position cur = depot;          // onde o camiao esta agora
    std::string curName = "D";
    for(size_t i = 0; i < stops.size(); i++){
        Position t = stops[i];     // proxima paragem
        int dx = t.x - cur.x, dy = t.y - cur.y;

        std::cout << i+1 << ". " << curName << "(" << cur.x << "," << cur.y << ") -> "
                  << names[i] << "(" << t.x << "," << t.y << "): ";
        if(dx > 0) std::cout << dx << " para a direita  ";
        if(dx < 0) std::cout << -dx << " para a esquerda  ";
        if(dy > 0) std::cout << dy << " para baixo  ";
        if(dy < 0) std::cout << -dy << " para cima  ";
        std::cout << "[" << std::abs(dx) + std::abs(dy) << " celulas]\n";

        // o destino passa a ser o ponto de partida do troco seguinte
        cur = t;
        curName = names[i];
    }
}


int main(){
    // 1) Posicao do deposito (ponto de partida e de chegada do camiao)
    Position depot;
    std::cout << "Posicao do deposito (dois numeros entre 0 e " << GRID-1 << ", ex: 0 0): ";
    while(!readPosition(depot)){
        std::cout << "Posicao invalida (escreve x e y, ex: 5 7): ";
    }

    // 2) Numero de caixotes
    std::cout << "Quantos caixotes? ";
    int n = readInt();
    while(n < 0){
        std::cout << "Numero invalido, tenta outra vez: ";
        n = readInt();
    }

    // 3) Posicao de cada caixote.
    //    Usa-se a classe Bin dos colegas (Environment.hpp). O tipo e a
    //    capacidade (General, 100.0) ainda nao contam para a rota.
    std::vector<Bin*> bins;
    for(int i = 0; i < n; i++){
        Position p;
        std::cout << "Caixote " << i+1 << " (x y): ";
        while(!readPosition(p)){
            std::cout << "Posicao invalida (escreve x e y, ex: 5 7): ";
        }
        bins.push_back(new Bin(i+1, p, BinType::General, 100.0));
    }

    // 4) Calculo da rota
    RoutePlanner planner(depot);
    std::vector<Bin*> order = planner.planOrder(bins);      // ordem de visita
    std::vector<Position> path = planner.buildPath(order);  // celulas percorridas

    // 5) Resultados
    std::cout << "\nOrdem de recolha: D";
    for(Bin* b : order){
        Position p = b->getPosition();
        std::cout << " -> C" << b->getId() << "(" << p.x << "," << p.y << ")";
    }
    std::cout << " -> D\n";
    std::cout << "Distancia total: " << planner.totalDistance(order) << " celulas\n\n";

    std::cout << "Percurso passo a passo:\n";
    printSteps(depot, order);

    std::cout << "\nMapa (D = deposito, 1,2,3... = ordem de recolha, setas = sentido do camiao):\n";
    printGrid(depot, order, path);

    // 6) Os Bin foram criados com new, por isso tem de ser libertados com delete
    for(Bin* b : bins) delete b;
    return 0;
}