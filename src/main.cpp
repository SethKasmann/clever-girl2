#include <iostream>
#include <vector>
#include "board.h"
#include "fen.h"
#include "move_list.h"
#include "move.h"
#include "move_generator.h"
#include "perft.h"

int main(int argc, char* argv[])
{
    std::cout << std::is_pod<Board>::value << '\n';
    MoveGenerator::init();
    bitboard::init();
    perft_fast();
    int z;
    std::cin >> z;
    return 0;
}