#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include <assert.h>
#include <nmmintrin.h>
#include <string>

#include "assert.h"
#include "magic_moves.h"

namespace bitboard
{
    constexpr uint64_t a_file = 0x8080808080808080;
    constexpr uint64_t h_file = 0x0101010101010101;

    constexpr uint64_t rank_1 = 0x00000000000000ff;
    constexpr uint64_t rank_2 = 0x000000000000ff00;
    constexpr uint64_t rank_3 = 0x0000000000ff0000;
    constexpr uint64_t rank_6 = 0x0000ff0000000000;
    constexpr uint64_t rank_7 = 0x00ff000000000000;
    constexpr uint64_t rank_8 = 0xff00000000000000;

    static uint64_t rook_attacks[64];
    static uint64_t bishop_attacks[64];
    static uint64_t between_mask[64][64];

    inline uint64_t between_horizonal(int square0, int square1)
    {
        return between_mask[square0][square1] & rook_attacks[square0];
    }

    inline uint64_t between_diagonal(int square0, int square1)
    {
        return between_mask[square0][square1] & bishop_attacks[square0];
    }

    inline uint64_t between(int square0, int square1)
    {
        return between_mask[square0][square1];
    }

    inline int get_lsb(uint64_t bitboard)
    {
        ASSERT(bitboard, bitboard, "Attempting to get_lsb of 0.");
        unsigned long index;
        if (_BitScanForward(&index, bitboard))
        {
            return static_cast<int>(index);
        }
        _BitScanForward(&index, bitboard >> 32);
        return static_cast<int>(index) + 32;
    }

    inline void pop_lsb(uint64_t& bitboard)
    {
        ASSERT(bitboard, bitboard, "Attempting to pop_lsb of 0.");
        bitboard &= bitboard - 1;
    }

    inline int pop_count(uint64_t bitboard)
    {
        return _mm_popcnt_u32(static_cast<unsigned int>(bitboard)) + _mm_popcnt_u32(static_cast<unsigned int>(bitboard >> 32));
    }

    inline int get_rank(uint64_t bitboard)
    {
        ASSERT(pop_count(bitboard) == 1, &bitboard, "Attempting to get_rank with a pop count != 1.");
        return get_lsb(bitboard) / 8;
    }

    inline uint64_t get_bitboard(int square)
    {
        ASSERT((square < 64), square, "Attempting to create a bitboard with a square > 63.");
        ASSERT((square >= 0), square, "Attempting to create a bitboard with a square < 0.");
        return 1ull << square;
    }

    inline void pretty(uint64_t bitboard)
    {
        const std::string bar = " +---+---+---+---+---+---+---+---+";

        std::cout << bar << std::endl;

        for (int i = 63; i >= 0; --i)
        {
            uint64_t square_bit = get_bitboard(i);
            if (square_bit & a_file)
            {
                std::cout << std::to_string(get_rank(square_bit) + 1);
            }
            std::cout << "| " << (square_bit & bitboard ? 1 : 0) << " ";
            if (square_bit & bitboard::h_file)
            {
                std::cout << '|' << std::endl << bar << std::endl;
            }
        }
    }

    inline void init()
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

#endif
