#ifndef MOVE_LIST_H
#define MOVE_LIST_H

#include <cstdlib>
#include <iostream>
#include <vector>

#include "bitboard.h"
#include "board.h"
#include "magic_moves.h"
#include "move.h"
#include "move_generator.h"

constexpr uint64_t promotion_mask = bitboard::rank_1 | bitboard::rank_8;

class MoveList {
private:
  std::array<Move, 255> _move_list;
  size_t _size;
public:
  MoveList(const Board &board) : _size(0) {
    // First push king moves.
    uint64_t attacked_squares = board.get_attack_mask(
        !board.player, board.get_occupied_mask() ^
                           board.get_piece_mask<Piece::king>(board.player));
    uint64_t valid_pieces =
        board.get_occupied_mask(board.player) & ~board.pinned;
    uint64_t valid_moves = ~board.get_occupied_mask(board.player);
    push_moves<Piece::king>(board, valid_pieces,
                            valid_moves & ~attacked_squares);

    if ((attacked_squares & board.get_piece_mask<Piece::king>(board.player)) !=
        0ull) {
      uint64_t check_mask = get_checks(board);
      // Return in the case of double check.
      if (bitboard::pop_count(check_mask) == 2) {
        return;
      }
      valid_moves =
          check_mask | bitboard::between(board.get_king_square(board.player),
                                         bitboard::get_lsb(check_mask));
    } else {
      // Push castle moves and pinned pieces moves only when the side to
      // move is not in check.
      generate_castle_moves(board, attacked_squares);
      generate_pinned_piece_moves(board, valid_moves);
    }

    // Normal Moves.
    all_pawn_moves(
        board, board.get_piece_mask<Piece::pawn>(board.player) & valid_pieces,
        valid_moves);
    push_moves<Piece::knight>(board, valid_pieces, valid_moves);
    push_moves<Piece::bishop>(board, valid_pieces, valid_moves);
    push_moves<Piece::rook>(board, valid_pieces, valid_moves);
    push_moves<Piece::queen>(board, valid_pieces, valid_moves);
  }

  size_t size() const { return _size; }

  Move get_move() {
    while (_size > 0) {
      return _move_list[--_size];
    }
    return null_move;
  }

  uint64_t get_checks(const Board &board) {
    int king_square = board.get_king_square(board.player);
    uint64_t checkers =
        (pseudo_pawn_attacks(board.player, king_square) &
         board.get_piece_mask<Piece::pawn>(!board.player)) |
        (attacks_from<Piece::knight>(king_square, board.get_occupied_mask()) &
         board.get_piece_mask<Piece::knight>(!board.player));
    uint64_t pseudo_attacks =
        (pseudo_rook_moves(king_square) &
         board.get_piece_mask<Piece::rook, Piece::queen>(!board.player)) |
        (pseudo_bishop_moves(king_square) &
         board.get_piece_mask<Piece::bishop, Piece::queen>(!board.player));
    while (pseudo_attacks) {
      int slider = bitboard::pop_lsb(pseudo_attacks);
      if ((bitboard::between(slider, king_square) &
           board.get_occupied_mask()) == 0ull) {
        checkers |= bitboard::get_bitboard(slider);
      }
    }
    return checkers;
  }

  void generate_pinned_piece_moves(const Board &board, uint64_t valid) {
    int king_square = board.get_king_square(board.player);
    uint64_t pinners = board.pinners;
    while (pinners) {
      int slider_square = bitboard::pop_lsb(pinners);
      int pinned_square = bitboard::get_lsb(
          bitboard::between(king_square, slider_square) & board.pinned);
      Piece pinned = board.get_piece(pinned_square);
      bool is_diagonal = (pseudo_bishop_moves(pinned_square) &
                          bitboard::get_bitboard(slider_square)) != 0ull;

      if (pinned == Piece::pawn) {
        uint64_t moves = is_diagonal
                             ? bitboard::get_bitboard(slider_square)
                             : bitboard::between(king_square, slider_square);
        all_pawn_moves(board, bitboard::get_bitboard(pinned_square),
                       moves & valid);
      } else if ((pinned == Piece::bishop && is_diagonal) ||
                 (pinned == Piece::rook && !is_diagonal) ||
                 pinned == Piece::queen) {
        uint64_t moves = (bitboard::between(king_square, slider_square) |
                          bitboard::get_bitboard(slider_square)) &
                         valid;
        while (moves) {
          _move_list[_size++] = {pinned_square, bitboard::pop_lsb(moves),
                                 Piece::none};
        }
      }
    }
  }

  void push_pawn_moves(uint64_t mask, int delta) {
    // Add pawn promotions to the move list.
    uint64_t move_mask = mask & promotion_mask;
    while (move_mask) {
      int to_square = bitboard::pop_lsb(move_mask);
      int from_square = to_square - delta;

      _move_list[_size++] = {from_square, to_square, Piece::queen};
      _move_list[_size++] = {from_square, to_square, Piece::bishop};
      _move_list[_size++] = {from_square, to_square, Piece::rook};
      _move_list[_size++] = {from_square, to_square, Piece::knight};
    }

    // Add non promotions to the move list.
    move_mask = mask & ~promotion_mask;
    while (move_mask) {
      int square = bitboard::pop_lsb(move_mask);
      _move_list[_size++] = {square - delta, square, Piece::none};
    }
  }

  bool en_passant_causes_check(const Board &board, int from, int ep_capture) {
    int king_square = board.get_king_square(board.player);
    // Confirm removing the captured pawn doesn't create a slider checker
    // along a diagonal.
    uint64_t checker =
        attacks_from<Piece::bishop>(ep_capture, board.get_occupied_mask()) &
        pseudo_bishop_moves(king_square) &
        board.get_piece_mask<Piece::bishop, Piece::queen>(!board.player);
    if (checker != 0ull) {
      return true;
    }
    uint64_t new_occupancy =
        board.get_occupied_mask() ^ bitboard::get_bitboard(from);
    // Confirm removing the attacking pawn and captured pawn doesn't
    // create a slider checker along the same rank.
    if (board.on_same_row(king_square, from) &&
        (bitboard::between_horizonal(ep_capture, king_square) &
         new_occupancy) == 0ull) {
      checker = attacks_from<Piece::rook>(ep_capture, new_occupancy) &
                pseudo_rook_moves(from) &
                board.get_piece_mask<Piece::rook, Piece::queen>(!board.player);
      if (checker != 0ull) {
        return true;
      }
    }
    return false;
  }

  void push_en_passant(const Board &board, uint64_t pawns, uint64_t valid,
                       int delta) {
    if (board.en_passant == 0) {
      return;
    }
    int captured = board.en_passant - delta;
    if ((bitboard::get_bitboard(captured) & valid) != 0ull) {
      uint64_t moves = pseudo_pawn_attacks(!board.player, board.en_passant) & pawns;
      if (moves != 0ull &&
          !en_passant_causes_check(board, captured, bitboard::get_lsb(moves))) {
        while (moves) {
          _move_list[_size++] = {bitboard::pop_lsb(moves), board.en_passant,
                                 Piece::none};
        }
      }
    }
  }

  void all_pawn_moves(const Board &board, uint64_t pawns, uint64_t valid) {
    const uint64_t valid_attacks =
        valid & board.get_occupied_mask(!board.player);
    const uint64_t valid_quiets = valid & board.get_empty_mask();
    uint64_t moves = 0ull;

    if (board.player == Player::white) {
      moves = pawns << 8 & valid_quiets;
      push_pawn_moves(moves, 8);
      moves = (pawns << 8 & board.get_empty_mask() & bitboard::rank_3) << 8 &
              valid_quiets;
      push_pawn_moves(moves, 16);
      moves = (pawns & ~bitboard::h_file) << 7 & valid_attacks;
      push_pawn_moves(moves, 7);
      moves = (pawns & ~bitboard::a_file) << 9 & valid_attacks;
      push_pawn_moves(moves, 9);
      push_en_passant(board, pawns, valid, 8);
    } else {
      moves = pawns >> 8 & valid_quiets;
      push_pawn_moves(moves, -8);
      moves = (pawns >> 8 & board.get_empty_mask() & bitboard::rank_6) >> 8 &
              valid_quiets;
      push_pawn_moves(moves, -16);
      moves = (pawns & ~bitboard::h_file) >> 9 & valid_attacks;
      push_pawn_moves(moves, -9);
      moves = (pawns & ~bitboard::a_file) >> 7 & valid_attacks;
      push_pawn_moves(moves, -7);
      push_en_passant(board, pawns, valid, -8);
    }
  }

  template <Piece Stm>
  void push_moves(const Board &board, uint64_t valid_piece_mask,
                  uint64_t valid) {
    uint64_t piece_mask =
        board.get_piece_mask<Stm>(board.player) & valid_piece_mask;
    while (piece_mask) {
      int from_square = bitboard::pop_lsb(piece_mask);
      uint64_t move_mask =
          attacks_from<Stm>(from_square, board.get_occupied_mask()) & valid;
      while (move_mask) {
        int to_square = bitboard::pop_lsb(move_mask);
        _move_list[_size++] = {from_square, to_square, Piece::none};
      }
    }
  }

  void generate_castle_moves(const Board &board,
                             uint64_t opponent_attack_mask) {
    int king_square = board.get_king_square(board.player);
    if (board.can_castle_kingside(opponent_attack_mask)) {
      _move_list[_size++] = {king_square, king_square - 2, Piece::none};
    }
    if (board.can_castle_queenside(opponent_attack_mask)) {
      _move_list[_size++] = {king_square, king_square + 2, Piece::none};
    }
  }
};

#endif