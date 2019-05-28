#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include <nmmintrin.h>
#include <string>

#include "assert.h"
#include "magic_moves.h"
#include "piece.h"
#include "player.h"

namespace bitboard
{
    constexpr uint64_t a_file = 0x8080808080808080;
    constexpr uint64_t b_file = 0x4040404040404040;
    constexpr uint64_t c_file = 0x2020202020202020;
    constexpr uint64_t d_file = 0x1010101010101010;
    constexpr uint64_t e_file = 0x0808080808080808;
    constexpr uint64_t f_file = 0x0404040404040404;
    constexpr uint64_t g_file = 0x0202020202020202;
    constexpr uint64_t h_file = 0x0101010101010101;

    constexpr uint64_t rank_1 = 0x00000000000000ff;
    constexpr uint64_t rank_2 = 0x000000000000ff00;
    constexpr uint64_t rank_3 = 0x0000000000ff0000;
    constexpr uint64_t rank_6 = 0x0000ff0000000000;
    constexpr uint64_t rank_7 = 0x00ff000000000000;
    constexpr uint64_t rank_8 = 0xff00000000000000;

    extern uint64_t rook_attacks[64];
    extern uint64_t bishop_attacks[64];
    extern uint64_t between_mask[64][64];

    uint64_t between_horizonal(int square0, int square1);
    uint64_t between_diagonal(int square0, int square1);
    uint64_t between(int square0, int square1);
    uint64_t pawn_attacks(Player player, uint64_t mask);
    uint64_t bishop_moves(int square);
    uint64_t rook_moves(int square);

    inline int get_lsb(uint64_t bitboard)
    {
        ASSERT(bitboard, bitboard, "Attempting to get_lsb of 0.");
        unsigned long index;
        if (_BitScanForward(&index, static_cast<unsigned long>(bitboard)))
        {
            return static_cast<int>(index);
        }
        _BitScanForward(&index, bitboard >> 32);
        return static_cast<int>(index) + 32;
    }

    inline int pop_lsb(uint64_t& bitboard)
    {
        ASSERT(bitboard, bitboard, "Attempting to pop_lsb of 0.");
        int lsb = get_lsb(bitboard);
        bitboard &= bitboard - 1;
        return lsb;
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

    inline uint64_t to_bitboard(int square)
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
            uint64_t square_bit = to_bitboard(i);
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

    void init();

}

#endif
