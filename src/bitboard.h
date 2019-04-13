#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include <assert.h>
#include <nmmintrin.h>

#include "assert.h"

namespace bitboard
{
	constexpr uint64_t a_file = 0x8080808080808080;
	constexpr uint64_t h_file = 0x0101010101010101;

	constexpr uint64_t rank_1 = 0x00000000000000ff;
	constexpr uint64_t rank_2 = 0x000000000000ff00;
	constexpr uint64_t rank_7 = 0x00ff000000000000;
	constexpr uint64_t rank_8 = 0xff00000000000000;

	inline int get_lsb(uint64_t bitboard)
	{
		ASSERT(bitboard, "Attempting to get_lsb of 0.");
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
		ASSERT(bitboard, "Attempting to pop_lsb of 0.");
		bitboard &= bitboard - 1;
	}

	inline int pop_count(uint64_t bitboard)
	{
		return _mm_popcnt_u32(static_cast<unsigned int>(bitboard)) + _mm_popcnt_u32(static_cast<unsigned int>(bitboard >> 32));
	}

	inline int get_rank(uint64_t bitboard)
	{
		ASSERT(pop_count(bitboard) == 1, "Attempting to get_rank with a pop count != 1.");
		return get_lsb(bitboard) / 8;
	}

	inline uint64_t get_bitboard(int square)
	{
		ASSERT(square < 64, "Attempting to create a bitboard with a square > 63.");
		ASSERT(square >= 0, "Attempting to create a bitboard with a square < 0.");
		return 1ull << square;
	}
}

#endif
