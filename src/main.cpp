#include <iostream>
#include "board.h"
#include "fen.h"
#include "move_list.h"
#include "move.h"
#include "move_generator.h"

struct PerftResult
{
    int nodes;
    int captures;
    int ep;
    int castles;
    int checks;
    PerftResult& operator+=(const PerftResult& perft_result)
    {
        nodes += perft_result.nodes;
        captures += perft_result.captures;
        ep += perft_result.ep;
        castles += perft_result.castles;
        checks += perft_result.checks;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& o, const PerftResult& perft_result)
{
    std::cout << "< PerftResult " << &perft_result
        << " nodes: " << perft_result.nodes << ", "
        << " captures: " << perft_result.captures << ", "
        << " ep: " << perft_result.ep << ", "
        << " castles: " << perft_result.castles << ", "
        << " checks: " << perft_result.checks << " >";
    return o;
}

PerftResult perft(Board& board, int depth)
{
    if (depth == 0)
    {
        return { 1, 0, 0, 0, 0 };
    }
    PerftResult result = { 0, 0, 0, 0, 0 };
    MoveList move_list = MoveList(board);
    while (1)
    {
        Move move = move_list.get_move(board);
        if (move == null_move)
        {
            break;
        }
        Board new_board = board;
        new_board.make_move(move);
        if (new_board.is_illegal())
        {
            continue;
        }
        if (depth == 1 && board.board[move.to] != Piece::none)
        {
            result.captures += 1;
        }
        if (bitboard::get_bitboard(move.to) == board.en_passant && board.get_piece(move.from) == Piece::pawn)
        {
            result.ep += 1;
        }
        //std::cout << new_board << std::endl;
        if (depth == 1)
        {
            if (new_board.is_attacked(bitboard::get_lsb(new_board.get_piece_mask(Piece::king, new_board.player)), new_board.player))
            {
                result.checks += 1;
            }
            if (move.castle)
            {
                result.castles += 1;
            }
        }
        result += perft(new_board, depth - 1);
    }
    return result;
}

int main(int argc, char* argv[])
{
    MoveGenerator::init();
    std::cout << std::is_pod<Board>::value << std::endl;
    Board board = fen::create_board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    PerftResult result = perft(board, 2);
    std::cout << result << std::endl;
    int z;
    std::cin >> z;
    return 0;
}