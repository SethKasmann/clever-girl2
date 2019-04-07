#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <string>
#include <array>

#include "bitboard.h"

struct Board
{
	void init();
	bool wtm;
	std::array<std::array<uint64_t, 6>, 2> pieces;
	std::array<uint64_t, 2> occupancy;
	uint64_t en_passant;
	int castle_rights;
	int halfmove_clock;
	int fullmove_number;
};

inline std::ostream& operator<<(std::ostream& o, const Board& board)
{
	const std::array<const std::array<char, 6>, 2> piece_chars
	{ {
		{ 'P', 'N', 'B', 'R', 'Q', 'K' },
		{ 'p', 'n', 'b', 'r', 'q', 'k' }
	} };

	const std::string bar = " +---+---+---+---+---+---+---+---+";

	o << bar << std::endl;

	for (uint64_t square_bit = 1ull << 63; square_bit > 0; square_bit >>= 1)
	{
		if (square_bit & bitboard::a_file)
		{
			o << std::to_string(bitboard::get_rank(square_bit) + 1);
		}
		o << '|' << (square_bit & board.en_passant ? '*' : ' ');
		if (square_bit & ~(board.occupancy[0] | board.occupancy[1]))
		{
			o << "  ";
		}
		for (int color = 0; color < 2; ++color)
		{
			for (int piece = 0; piece < 6; ++piece)
			{
				if (board.pieces[color][piece] & square_bit)
				{
					o << piece_chars[color][piece] << ' ';
				}
			}
		}
		if (square_bit & bitboard::h_file)
		{
			o << '|' << std::endl << bar << std::endl;
		}
	}

	o << "  A   B   C   D   E   F   G   H" << std::endl;

	o << (board.wtm ? "White" : "Black");
	o << " to move." << std::endl;

	o << "Castle Rights: ";
	if (board.castle_rights & 1ull)
	{
		o << "K";
	}
	if (board.castle_rights & 1ull << 1)
	{
		o << "Q";
	}
	if (board.castle_rights & 1ull << 2)
	{
		o << "k";
	}
	if (board.castle_rights & 1ull << 3)
	{
		o << "q";
	}
	o << std::endl;

	return o;
}

#endif
