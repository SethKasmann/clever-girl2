#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <vector>
#include <iostream>
#include <cstdlib>
#include <functional>

#include "board.h"
#include "move.h"
#include "bitboard.h"
#include "magic_moves.h"

using Generator = std::function<uint64_t(uint64_t, int)>;

constexpr uint64_t promotion_mask = bitboard::rank_1 | bitboard::rank_8;

class MoveList
{
private:
	Board _board;
	std::vector<Move> _move_list;
public:
	MoveList(const Board& board)
		: _board(board)
	{
		generate_pawn_moves();
	}

	void push_pawn_moves(uint64_t mask, int delta)
	{
		// Add pawn promotions to the move list.
		uint64_t promotion_mask = mask & promotion_mask;
		for (; promotion_mask; bitboard::pop_lsb(promotion_mask))
		{
			int square = bitboard::get_lsb(promotion_mask);
			_move_list.push_back({ square - delta, square, 4, 0 });
			_move_list.push_back({ square - delta, square, 3, 0 });
			_move_list.push_back({ square - delta, square, 2, 0 });
			_move_list.push_back({ square - delta, square, 1, 0 });
		}

		// Add non promotions to the move list.
		uint64_t move_mask = mask & ~promotion_mask;
		for (; move_mask; bitboard::pop_lsb(move_mask))
		{
			int square = bitboard::get_lsb(move_mask);
			_move_list.push_back({ square - delta, square, 0, 0 });
		}
	}

	uint64_t move_pawns(uint64_t from_mask, uint64_t to_mask, int delta, Generator generator)
	{
		return generator(from_mask, std::abs(delta)) & to_mask;
	}

	void push_all_pawn_moves(int forward_delta, Generator generator)
	{
		const uint64_t pawn_mask = _board.pieces[_board.wtm][0];
		const uint64_t empty_mask = ~(_board.occupancy[0] | _board.occupancy[1]);
		const uint64_t enemy_mask = _board.occupancy[!_board.wtm] | _board.en_passant;

		// Single pawn push.
		uint64_t single_push_mask = move_pawns(pawn_mask, empty_mask, forward_delta, generator);
		push_pawn_moves(single_push_mask, forward_delta);

		// Double pawn push.
		uint64_t double_push_mask = move_pawns(single_push_mask, empty_mask, forward_delta, generator);
		push_pawn_moves(double_push_mask, forward_delta);

		// Pawn attacks left.
		uint64_t pawn_left_attack_mask = move_pawns(pawn_mask & ~bitboard::a_file, enemy_mask, forward_delta + 1, generator);
		push_pawn_moves(pawn_left_attack_mask, forward_delta + 1);

		// Pawn attacks right.
		uint64_t pawn_right_attack_mask = move_pawns(pawn_mask & ~bitboard::h_file, enemy_mask, forward_delta - 1, generator);
		push_pawn_moves(pawn_right_attack_mask, forward_delta - 1);
	}

	void generate_pawn_moves()
	{
		// Left shift is used for white and right shift is used for black.
		const auto generator = _board.wtm 
			? std::function<uint64_t(uint64_t, int)>{[](uint64_t mask, int shift) { return mask << shift; }}
			: std::function<uint64_t(uint64_t, int)>{[](uint64_t mask, int shift) { return mask >> shift; }};

		// The amount of shift needed for a pawn push from white's point of view.
		const int forward_delta = _board.wtm ? 8 : -8;

		push_all_pawn_moves(forward_delta, generator);
	}

	void push_moves(uint64_t piece_mask, uint64_t occupancy, Generator generator)
	{
		for (; piece_mask; bitboard::pop_lsb(piece_mask))
		{
			int from_square = bitboard::get_lsb(piece_mask);
			uint64_t move_mask = generator(occupancy, from_square);
			for (; move_mask; bitboard::pop_lsb(move_mask))
			{
				int to_square = bitboard::get_lsb(move_mask);
				_move_list.push_back({ from_square, to_square, 0, 0 });
			}
		}
	}

	void generate_piece_moves()
	{
		uint64_t occupancy_mask = _board.occupancy[0] | _board.occupancy[1];
		push_moves(_board.pieces[_board.wtm][1], occupancy_mask, [](uint64_t occupancy, int square) { return Nmagic(square, occupancy); });
		push_moves(_board.pieces[_board.wtm][2], occupancy_mask, [](uint64_t occupancy, int square) { return Bmagic(square, occupancy); });
		push_moves(_board.pieces[_board.wtm][3], occupancy_mask, [](uint64_t occupancy, int square) { return Rmagic(square, occupancy); });
		push_moves(_board.pieces[_board.wtm][4], occupancy_mask, [](uint64_t occupancy, int square) { return Qmagic(square, occupancy); });
	}

};

#endif