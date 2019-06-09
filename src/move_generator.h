#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <iostream>
#include <string>

#include "magic_moves.h"
#include "player.h"
#include "piece.h"
#include "bitboard.h"
#include "board.h"

uint64_t pseudo_pawn_moves(Player player, int square);
uint64_t pseudo_pawn_quiets(Player player, int square);
uint64_t pseudo_pawn_attacks(Player player, int square);
uint64_t pseudo_knight_moves(int square);
uint64_t pseudo_bishop_moves(int square);
uint64_t pseudo_rook_moves(int square);
uint64_t pseudo_queen_moves(int square);
uint64_t pseudo_king_moves(int square);

template<Player Stm>
class MoveGen
{
private:
    uint64_t _occupancy;
public:
    MoveGen() = default;
    explicit MoveGen(uint64_t occupancy) noexcept
        : _occupancy(occupancy)
    {}
    uint64_t pawn_attacks_left(uint64_t mask) const noexcept
    {
        return _occupancy & (Stm == Player::white ? (mask & ~bitboard::a_file) << 9 : (mask & ~bitboard::h_file) >> 9);
    }
    uint64_t pawn_attacks_right(uint64_t mask) const noexcept
    {
        return _occupancy & (Stm == Player::white ? (mask & ~bitboard::h_file) << 7 : (mask & ~bitboard::a_file) >> 7);
    }
    uint64_t pawn_push(uint64_t mask) const noexcept
    {
        return ~_occupancy & (Stm == Player::white ? mask << 8 : mask >> 8);
    }
    uint64_t pawn_double_push(uint64_t mask) const noexcept
    {
        return ~_occupancy & (pawn_push(pawn_push(mask) & ~_occupancy & PlayerTraits<Stm>::double_mask));
    }
    template<Piece P>
    uint64_t attacks_from(int square, uint64_t valid=ULONG_MAX) const
    {
        switch (P)
        {
        case Piece::pawn:
            return pseudo_pawn_attacks(Stm, square) & valid;
        case Piece::knight:
            return pseudo_knight_moves(square) & valid;
        case Piece::bishop:
            if ((pseudo_bishop_moves(square) & valid) == 0u) {
                return 0u;
            }
            return Bmagic(square, _occupancy) & valid;
        case Piece::rook:
            if ((pseudo_rook_moves(square) & valid) == 0u) {
                return 0u;
            }
            return Rmagic(square, _occupancy) & valid;
        case Piece::queen:
            if ((pseudo_queen_moves(square) & valid) == 0u) {
                return 0u;
            }
            return Qmagic(square, _occupancy) & valid;
        case Piece::king:
            return pseudo_king_moves(square) & valid;
        }
    }

    template<Piece P>
    uint64_t attacks_by(uint64_t pieces) const
    {
        if (P == Piece::pawn)
        {
            return pawn_attacks_left(pieces) | pawn_attacks_right(pieces);
        }
        uint64_t attacks = 0u;
        while (pieces)
        {
            attacks |= attacks_from<P>(bitboard::pop_lsb(pieces));
        }
        return attacks;
    }
};

inline uint64_t pawn_attacks_left(Player player, uint64_t mask)
{
    return player == Player::white ? (mask & ~bitboard::a_file) << 9 : (mask & ~bitboard::h_file) >> 9;
}

inline uint64_t pawn_attacks_right(Player player, uint64_t mask)
{
    return player == Player::white ? (mask & ~bitboard::h_file) << 7 : (mask & ~bitboard::a_file) >> 7;
}

inline uint64_t pawn_push(Player player, uint64_t mask)
{
    return player == Player::white ? mask << 8 : mask >> 8;
}

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
    return pawn_attacks(player, bitboard::to_bitboard(square));
}

template<Piece Stm> uint64_t attacks_from(int square, uint64_t occupancy);

template<> inline uint64_t attacks_from<Piece::knight>(int square, uint64_t occupancy)
{
    return pseudo_knight_moves(square);
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
    return pseudo_king_moves(square);
}

template<Piece Stm> uint64_t pseudo_attacks_from(int square);

template<> inline uint64_t pseudo_attacks_from<Piece::knight>(int square)
{
    return pseudo_knight_moves(square);
}

template<> inline uint64_t pseudo_attacks_from<Piece::bishop>(int square)
{
    return pseudo_bishop_moves(square);
}

template<> inline uint64_t pseudo_attacks_from<Piece::rook>(int square)
{
    return pseudo_rook_moves(square);
}

template<> inline uint64_t pseudo_attacks_from<Piece::queen>(int square)
{
    return pseudo_queen_moves(square);
}

template<> inline uint64_t pseudo_attacks_from<Piece::king>(int square)
{
    return pseudo_king_moves(square);
}

void move_generator_init();

#endif