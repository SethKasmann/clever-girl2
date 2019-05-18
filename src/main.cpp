#include <iostream>
#include <vector>
#include "board.h"
#include "fen.h"
#include "move.h"
#include "move_generator.h"
#include "perft.h"
#include "hash.h"

int main(int argc, char* argv[])
{
    std::cout << std::is_pod<Board>::value << '\n';
    move_generator_init();
    bitboard::init();
    hash_init();
    perft_fast();
    int z;
    std::cin >> z;
    return 0;
}