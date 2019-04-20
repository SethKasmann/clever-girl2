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

constexpr unsigned kingside_castle_white = 1;
constexpr unsigned queenside_castle_white = 2;
constexpr unsigned kingside_castle_black = 4;
constexpr unsigned queenside_castle_black = 8;

inline bool can_castle_kingside(Board board)
{
	bool can_castle = false;
	if (board.castle_rights & (board.player == Player::white ? kingside_castle_white : kingside_castle_black))
	{
		uint64_t empty_mask = board.get_empty_mask();
		uint64_t king_mask = board.get_piece_mask(Piece::king, board.player);

		can_castle = empty_mask >> 1 & king_mask && empty_mask >> 2 & king_mask;
	}
	return can_castle;
}

inline bool can_castle_queenside(Board board)
{
	bool can_castle = false;
	if (board.castle_rights & (board.player == Player::white ? queenside_castle_white : queenside_castle_black))
	{
		uint64_t empty_mask = board.get_empty_mask();
		uint64_t king_mask = board.get_piece_mask(Piece::king, board.player);

		can_castle = empty_mask << 1 & king_mask && empty_mask << 2 & king_mask && empty_mask << 3 & king_mask;
	}
	return can_castle;
}

struct MoveGenerator
{
	template<Piece P>
	static uint64_t generate_moves(uint64_t occupancy_mask, int square, Player player=Player::white)
	{
		switch (P)
		{
			case Piece::pawn:
				return Pmagic(square, occupancy_mask, player);
			case Piece::knight:
				return Nmagic(square, occupancy_mask);
			case Piece::bishop:
				return Bmagic(square, occupancy_mask);
			case Piece::rook:
				return Rmagic(square, occupancy_mask);
			case Piece::queen:
				return Qmagic(square, occupancy_mask);
			case Piece::king:
				return Kmagic(square, occupancy_mask);
		}
	}
};

class MoveList
{
private:
	std::vector<Move> _move_list;
	uint64_t _check_mask;
	uint64_t _pin_mask;
public:
	MoveList(const Board& board)
	{
		set_check_mask(board);
		generate_pawn_moves(board);
		generate_piece_moves(board);
		generate_castle_moves(board);
	}

	Move get_move()
	{
		Move next_move = null_move;

		if (_move_list.size() > 0)
		{
			next_move = _move_list.back();
			_move_list.pop_back();
		}

		return next_move;
	}

	void set_check_mask(const Board& board)
	{
		_check_mask = 0ull;

		int king_square = bitboard::get_lsb(board.get_piece_mask(Piece::king, board.player));

		_check_mask |= MoveGenerator::generate_moves<Piece::pawn>(board.get_occupied_mask(), king_square, board.player) & board.get_piece_mask(Piece::pawn, !board.player);
		_check_mask |= MoveGenerator::generate_moves<Piece::knight>(board.get_occupied_mask(), king_square, board.player) & board.get_piece_mask(Piece::knight, !board.player);
		_check_mask |= MoveGenerator::generate_moves<Piece::bishop>(board.get_occupied_mask(), king_square, board.player) & board.get_piece_mask(Piece::bishop, !board.player);
		_check_mask |= MoveGenerator::generate_moves<Piece::rook>(board.get_occupied_mask(), king_square, board.player) & board.get_piece_mask(Piece::rook, !board.player);
		_check_mask |= MoveGenerator::generate_moves<Piece::queen>(board.get_occupied_mask(), king_square, board.player) & board.get_piece_mask(Piece::queen, !board.player);
	}

	void push_pawn_moves(uint64_t mask, int delta)
	{
		// Add pawn promotions to the move list.
		uint64_t move_mask = mask & promotion_mask;
		for (; move_mask; bitboard::pop_lsb(move_mask))
		{
			int to_square = bitboard::get_lsb(move_mask);
			int from_square = to_square - delta;

			_move_list.push_back({ from_square, to_square, Piece::queen, false });
			_move_list.push_back({ from_square, to_square, Piece::bishop, false });
			_move_list.push_back({ from_square, to_square, Piece::rook, false });
			_move_list.push_back({ from_square, to_square, Piece::knight, false });
		}

		// Add non promotions to the move list.
		move_mask = mask & ~promotion_mask;
		for (; move_mask; bitboard::pop_lsb(move_mask))
		{
			int square = bitboard::get_lsb(move_mask);
			_move_list.push_back({ square - delta, square, Piece::none, false });
		}
	}

	uint64_t move_pawns(uint64_t from_mask, uint64_t to_mask, int delta, Generator generator)
	{
		return generator(from_mask, std::abs(delta)) & to_mask;
	}

	void push_all_pawn_moves(const Board& board, int forward_delta, Generator generator)
	{
		const uint64_t pawn_mask = board.get_piece_mask(Piece::pawn, board.player);
		const uint64_t empty_mask = board.get_empty_mask();
		const uint64_t enemy_mask = board.occupancy[!board.player] | board.en_passant;

		// Single pawn push.
		uint64_t single_push_mask = move_pawns(pawn_mask, empty_mask, forward_delta, generator);
		push_pawn_moves(single_push_mask, forward_delta);

		// Double pawn push.
		uint64_t double_push_mask = move_pawns(single_push_mask, empty_mask, forward_delta, generator);
		push_pawn_moves(double_push_mask, forward_delta * 2);

		// Pawn attacks left.
		uint64_t pawn_left_attack_mask = move_pawns(pawn_mask & ~bitboard::a_file, enemy_mask, forward_delta + 1, generator);
		push_pawn_moves(pawn_left_attack_mask, forward_delta + 1);

		// Pawn attacks right.
		uint64_t pawn_right_attack_mask = move_pawns(pawn_mask & ~bitboard::h_file, enemy_mask, forward_delta - 1, generator);
		push_pawn_moves(pawn_right_attack_mask, forward_delta - 1);
	}

	void generate_pawn_moves(const Board& board)
	{
		// Left shift is used for white and right shift is used for black.
		const auto generator = board.player == white
			? std::function<uint64_t(uint64_t, int)>{[](uint64_t mask, int shift) { return mask << shift; }}
			: std::function<uint64_t(uint64_t, int)>{[](uint64_t mask, int shift) { return mask >> shift; }};

		// The amount of shift needed for a pawn push from white's point of view.
		const int forward_delta = board.player == Player::white ? 8 : -8;

		push_all_pawn_moves(board, forward_delta, generator);
	}

	template<Piece P>
	void push_moves(const Board & board, uint64_t valid_mask)
	{
		uint64_t piece_mask = board.get_piece_mask(P, board.player);
		for (; piece_mask; bitboard::pop_lsb(piece_mask))
		{
			int from_square = bitboard::get_lsb(piece_mask);
			uint64_t move_mask = MoveGenerator::generate_moves<P>(board.get_occupied_mask(), from_square) & valid_mask;
			for (; move_mask; bitboard::pop_lsb(move_mask))
			{
				int to_square = bitboard::get_lsb(move_mask);
				_move_list.push_back({ from_square, to_square, Piece::none, false });
			}
		}
	}

	void generate_castle_moves(const Board& board)
	{
		uint64_t king_mask = board.get_piece_mask(Piece::king, board.player);
		uint64_t empty_mask = board.get_empty_mask();
		int from_square = bitboard::get_lsb(king_mask);

		if (can_castle_kingside(board))
		{
			_move_list.push_back({ from_square, from_square - 2, Piece::none, true });
		}
		if (can_castle_queenside(board))
		{
			_move_list.push_back({ from_square, from_square + 2, Piece::none, true });
		}
	}

	void generate_piece_moves(const Board& board)
	{
		uint64_t valid_mask = board.get_occupied_mask(!board.player) | board.get_empty_mask();

		push_moves<Piece::knight>(board, valid_mask);
		push_moves<Piece::bishop>(board, valid_mask);
		push_moves<Piece::rook>(board, valid_mask);
		push_moves<Piece::queen>(board, valid_mask);
		push_moves<Piece::king>(board, valid_mask);
	}

};

#endif