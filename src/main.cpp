#include <iostream>
#include "board.h"
#include "fen.h"
#include "move_list.h"
#include "move.h"
#include "move_generator.h"
#include "perft.h"

//struct PerftResult
//{
//    int nodes;
//    int captures;
//    int ep;
//    int castles;
//    PerftResult& operator+=(const PerftResult& perft_result)
//    {
//        nodes += perft_result.nodes;
//        captures += perft_result.captures;
//        ep += perft_result.ep;
//        castles += perft_result.castles;
//        return *this;
//    }
//};
//
//std::ostream& operator<<(std::ostream& o, const PerftResult& perft_result)
//{
//    std::cout << "< PerftResult " << &perft_result
//        << " nodes: " << perft_result.nodes << ", "
//        << " captures: " << perft_result.captures << ", "
//        << " ep: " << perft_result.ep << ", "
//        << " castles: " << perft_result.castles << " >";
//    return o;
//}
//
//PerftResult perft(const Board& board, int depth)
//{
//    if (depth == 0)
//    {
//        return { 1, 0, 0, 0 };
//    }
//    PerftResult result = { 0, 0, 0, 0 };
//    MoveList move_list = MoveList(board);
//    Move move = move_list.get_move(board);
//    for (; !(move == null_move); move = move_list.get_move(board))
//    {
//        if (depth == 1)
//        {
//            result.nodes++;
//            if (board.board[move.to] != Piece::none)
//            {
//                result.captures++;
//            }
//            else if (move.castle)
//            {
//                result.castles++;
//            }
//            else if (bitboard::get_bitboard(move.to) == board.en_passant && board.get_piece(move.from) == Piece::pawn)
//            {
//                result.ep++;
//            }
//            continue;
//        }
//        Board new_board = board;
//        new_board.make_move(move);
//        result += perft(new_board, depth - 1);
//    }
//    return result;
//}

int main(int argc, char* argv[])
{
    MoveGenerator::init();
    bitboard::init();
    perft_fast();
    int z;
    std::cin >> z;
    return 0;
}