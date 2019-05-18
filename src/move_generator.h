#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <iostream>

#include "magic_moves.h"
#include "player.h"
#include "piece.h"
#include "bitboard.h"

inline uint64_t pawn_attacks(Player player, uint64_t mask)
{
    if (player == Player::white)
    {
        return (mask & ~bitboard::h_file) << 7 | (mask & ~bitboard::a_file) << 9;
    }
    else
    {
        return (mask & ~bitboard::h_file) >> 9 | (mask & ~bitboard::a_file) >> 7;
    }
}

inline uint64_t pawn_attacks(Player player, int square)
{
    return pawn_attacks(player, bitboard::get_bitboard(square));
}

template<Piece P> uint64_t attacks_from(int square, uint64_t occupancy);

template<> inline uint64_t attacks_from<Piece::knight>(int square, uint64_t occupancy)
{
    return Nmagic(square, occupancy);
}

template<> inline uint64_t attacks_from<Piece::bishop>(int square, uint64_t occupancy)
{
    return Bmagic(square, occupancy);
}

template<> inline uint64_t attacks_from<Piece::rook>(int square, uint64_t occupancy)
{
    return Rmagic(square, occupancy);
}

template<> inline uint64_t attacks_from<Piece::queen>(int square, uint64_t occupancy)
{
    return Qmagic(square, occupancy);
}

template<> inline uint64_t attacks_from<Piece::king>(int square, uint64_t occupancy)
{
    return Kmagic(square, occupancy);
}

inline void move_generator_init()
{
    initmagicmoves();
}

//struct MoveGenerator
//{
//    template<Piece P>
//    static uint64_t generate_moves(uint64_t occupancy_mask, int square, Player player = Player::white)
//    {
//        static_assert(P >= Piece::pawn || P <= Piece::king);
//        switch (P)
//        {
//        case Piece::pawn:
//            return Pmagic(square, occupancy_mask, player);
//        case Piece::knight:
//            return Nmagic(square, occupancy_mask);
//        case Piece::bishop:
//            return Bmagic(square, occupancy_mask);
//        case Piece::rook:
//            return Rmagic(square, occupancy_mask);
//        case Piece::queen:
//            return Qmagic(square, occupancy_mask);
//        case Piece::king:
//            return Kmagic(square, occupancy_mask);
//        }
//    }

//    static void init()
//    {
//        initmagicmoves();
//    }
//};

#endif