#include "bitboard.h"

namespace bitboard
{
    uint64_t rook_attacks[64];
    uint64_t bishop_attacks[64];
    uint64_t between_mask[64][64];

    uint64_t between_horizonal(int square0, int square1)
    {
        return between_mask[square0][square1] & rook_attacks[square0];
    }

    uint64_t between_diagonal(int square0, int square1)
    {
        return between_mask[square0][square1] & bishop_attacks[square0];
    }

    uint64_t between(int square0, int square1)
    {
        return between_mask[square0][square1];
    }

    uint64_t pawn_attacks(Player player, uint64_t mask)
    {
        if (player == Player::white)
        {
            return (mask & ~h_file) << 7 | (mask & ~bitboard::a_file) << 9;
        }
        else
        {
            return (mask & ~h_file) >> 9 | (mask & ~a_file) >> 7;
        }
    }

    uint64_t bishop_moves(int square)
    {
        return bishop_attacks[square];
    }

    uint64_t rook_moves(int square)
    {
        return rook_attacks[square];
    }
    //uint64_t pseudo_attacks(Player player, Piece piece, int square)
    //{
    //    if (piece == Piece::pawn)
    //    {
    //        return pseudo_attacks_mask[piece][square]; //& in_front_of[player][square];
    //    }
    //    return pseudo_attacks_mask[piece][square];
    //}

    void init()
    {
        for (int i = 0; i < 64; ++i)
        {
            uint64_t i_mask = get_bitboard(i);
            rook_attacks[i] = Rmagic(i, 0ull);
            bishop_attacks[i] = Bmagic(i, 0ull);
            for (int j = 0; j < 64; ++j)
            {
                uint64_t j_mask = get_bitboard(j);
                // Set the between mask. QMagic cannot be used - two bits on the same horizonal plane may share
                // diagonal bits and vice versa.
                between_mask[i][j] = 0ull;
                if (rook_attacks[i] & j_mask)
                {
                    between_mask[i][j] |= Rmagic(i, i_mask | j_mask) & Rmagic(j, i_mask | j_mask);
                }
                if (bishop_attacks[i] & j_mask)
                {
                    between_mask[i][j] |= Bmagic(i, i_mask | j_mask) & Bmagic(j, i_mask | j_mask);
                }
            }
        }
    }
}