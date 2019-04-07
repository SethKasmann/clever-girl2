#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include <assert.h>
#include <nmmintrin.h>

#ifndef NDEBUG
#define ASSERT(condition, message) \
do \
{ \
	if (!condition) \
	{ \
		std::cerr << "Assertion \"" #condition "\" failed in " << __FILE__ \
			<< " on line " << __LINE__ << std::endl \
			<< message << std::endl; \
	} \
} while (false)
#else
#define ASSERT(condition, message) do {} while (false)
#endif

namespace bitboard
{
	constexpr uint64_t a_file = 0x8080808080808080;
	constexpr uint64_t h_file = 0x0101010101010101;

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

	inline int pop_count(uint64_t bitboard)
	{
		return _mm_popcnt_u32(static_cast<unsigned int>(bitboard)) + _mm_popcnt_u32(static_cast<unsigned int>(bitboard >> 32));
	}

	inline int get_rank(uint64_t bitboard)
	{
		ASSERT(pop_count(bitboard) == 1, "Attempting to get_rank with a pop count != 1.");
		return get_lsb(bitboard) / 8;
	}
}

#endif
