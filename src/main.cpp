#include <iostream>
#include "board.h"
#include "fen.h"
#include "move_generator.h"
#include "move.h"

int main(int argc, char* argv[])
{
    std::cout << std::is_pod<Board>::value << std::endl;
    Board b = fen::create_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    MoveList move_list = MoveList(b);
    while (1)
    {
        Move m = move_list.get_move();
        if (m == null_move)
        {
            break;
        }
        Board temp(b);
        temp.make_move(m);
        std::cout << temp;
    }
    //b.init_from_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
    std::cout << b;
    return 0;
}