#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include <assert.h>
#include <nmmintrin.h>
#include <string>

#include "assert.h"

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

        std::cout << bar;

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

    inline
        uint64_t north_fill(uint64_t gen)
    {
        gen |= (gen << 8);
        gen |= (gen << 16);
        gen |= (gen << 32);
        return gen;
    }

    inline
        uint64_t south_fill(uint64_t gen)
    {
        gen |= (gen >> 8);
        gen |= (gen >> 16);
        gen |= (gen >> 32);
        return gen;
    }

    inline
        uint64_t east_fill(uint64_t gen)
    {
        const uint64_t pr0 = ~h_file;
        const uint64_t pr1 = pr0 & (pr0 << 1);
        const uint64_t pr2 = pr1 & (pr1 << 2);
        gen |= pr0 & (gen << 1);
        gen |= pr1 & (gen << 2);
        gen |= pr2 & (gen << 4);
        return gen;
    }

    inline
        uint64_t north_east_fill(uint64_t gen)
    {
        const uint64_t pr0 = ~h_file;
        const uint64_t pr1 = pr0 & (pr0 << 9);
        const uint64_t pr2 = pr1 & (pr1 << 18);
        gen |= pr0 & (gen << 9);
        gen |= pr1 & (gen << 18);
        gen |= pr2 & (gen << 36);
        return gen;
    }

    inline
        uint64_t south_east_fill(uint64_t gen)
    {
        const uint64_t pr0 = ~h_file;
        const uint64_t pr1 = pr0 & (pr0 >> 7);
        const uint64_t pr2 = pr1 & (pr1 >> 14);
        gen |= pr0 & (gen >> 7);
        gen |= pr1 & (gen >> 14);
        gen |= pr2 & (gen >> 28);
        return gen;
    }

    inline
        uint64_t west_fill(uint64_t gen)
    {
        const uint64_t pr0 = ~a_file;
        const uint64_t pr1 = pr0 & (pr0 >> 1);
        const uint64_t pr2 = pr1 & (pr1 >> 2);
        gen |= pr0 & (gen >> 1);
        gen |= pr1 & (gen >> 2);
        gen |= pr2 & (gen >> 4);
        return gen;
    }

    inline
        uint64_t south_west_fill(uint64_t gen)
    {
        const uint64_t pr0 = ~a_file;
        const uint64_t pr1 = pr0 & (pr0 >> 9);
        const uint64_t pr2 = pr1 & (pr1 >> 18);
        gen |= pr0 & (gen >> 9);
        gen |= pr1 & (gen >> 18);
        gen |= pr2 & (gen >> 36);
        return gen;
    }

    inline
        uint64_t north_west_fill(uint64_t gen)
    {
        const uint64_t pr0 = ~a_file;
        const uint64_t pr1 = pr0 & (pr0 << 7);
        const uint64_t pr2 = pr1 & (pr1 << 14);
        gen |= pr0 & (gen << 7);
        gen |= pr1 & (gen << 14);
        gen |= pr2 & (gen << 28);
        return gen;
    }

    inline uint64_t get_diagonal_mask(int square)
    {
        uint64_t mask = get_bitboard(square);
        return north_east_fill(mask) | north_west_fill(mask) | south_east_fill(mask) | south_west_fill(mask);
    }

    inline uint64_t get_horizontal_mask(int square)
    {
        uint64_t mask = get_bitboard(square);
        return north_fill(mask) | south_fill(mask) | east_fill(mask) | west_fill(mask);
    }

}

#endif
