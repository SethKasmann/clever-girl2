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

template <Player Stm> class MoveList {
private:
  std::array<Move, 255> _move_list;
  size_t _size;
  MoveGen<Stm> _gen;
  uint64_t _checkers;
  uint64_t _attacks;
  uint64_t _pinned;
  uint64_t _contact_check;

public:
  MoveList(const Board &board)
      : _size(0), _gen(board.get_occupied_mask()), _checkers(0u), _attacks(0u),
        _pinned(0u), _contact_check(0u) {

    generate_pinned_piece_moves_again(board);
    uint64_t valid_moves = ~board.get_occupied_mask<Stm>();
    push_moves<Piece::king>(board, ~board.get_occupied_mask<Stm>() & ~_attacks);
    if (_checkers != 0u) {
      if (bitboard::pop_count(_checkers) == 2) {
        return;
      }
      valid_moves = _checkers | bitboard::between(board.get_king_square<Stm>(),
                                                  bitboard::get_lsb(_checkers));
    } else {
      generate_castle_moves(board);
    }

    // Normal Moves.
    all_pawn_moves(board, valid_moves);
    push_moves<Piece::knight>(board, valid_moves);
    push_moves<Piece::bishop>(board, valid_moves);
    push_moves<Piece::rook>(board, valid_moves);
  }

  size_t size() const { return _size; }

  Move get_move() {
    if (_size > 0) {
      return _move_list[--_size];
    }
    return null_move;
  }

  void push_king_moves(const Board &board) {
    const int square = board.get_king_square<Stm>();
    const uint64_t valid = ~board.get_occupied_mask<Stm>() & ~_attacks;

    if (_checkers != 0u) {
      // Empty the move list when if in check.
      _size = 0;
    } else {
      // Make castling moves when not in check.
      if (board.can_castle_kingside<Stm>(_attacks)) {
        _move_list[_size++] = {square, square - 2, Piece::none};
      }
      if (board.can_castle_queenside<Stm>(_attacks)) {
        _move_list[_size++] = {square, square + 2, Piece::none};
      }
    }
  }

  void generate_pinned_piece_moves_again(const Board &board) {
    const int king_square = board.get_king_square<Stm>();
    // Set the attack mask.
    _checkers |= pawn_attacks(Stm, king_square) &
                 board.get_piece_mask<!Stm, Piece::pawn>();
    _checkers |= pseudo_knight_moves(king_square) &
                 board.get_piece_mask<!Stm, Piece::knight>();
    _contact_check |= _checkers;
    _attacks |= pawn_attacks(!Stm, board.get_piece_mask<!Stm, Piece::pawn>());
    uint64_t knights = board.get_piece_mask<!Stm, Piece::knight>();
    while (knights) {
      _attacks |= pseudo_knight_moves(bitboard::pop_lsb(knights));
    }

    // Remove the king from occupancy.
    const uint64_t occupancy_wo_king =
        board.get_occupied_mask() ^ board.get_piece_mask<Stm, Piece::king>();
    uint64_t sliders =
        board.get_piece_mask<!Stm, Piece::bishop, Piece::queen>();
    while (sliders) {
      int slider_square = bitboard::pop_lsb(sliders);
      uint64_t slider_attacks =
          attacks_from<Piece::bishop>(slider_square, occupancy_wo_king);
      // See if the piece checks the king.
      if (slider_attacks & bitboard::to_bitboard(king_square)) {
        _checkers |= bitboard::to_bitboard(slider_square);
      }
      // Check if the enemy slider pins a piece against the stm king.
      else if (bitboard::pop_count(
                   bitboard::between_diagonal(slider_square, king_square) &
                   occupancy_wo_king) == 1 &&
               (bitboard::between_diagonal(slider_square, king_square) &
                board.get_occupied_mask<Stm>()) != 0u) {
        int pinned_square = bitboard::get_lsb(
            bitboard::between_diagonal(slider_square, king_square) &
            occupancy_wo_king);
        Piece pinned = board.get_piece(pinned_square);
        _pinned |= bitboard::to_bitboard(pinned_square);
        // Pinned pawns on a diagonal can only capture the piece.
        if (pinned == Piece::pawn) {
          generate_pawn_attacks(bitboard::to_bitboard(pinned_square),
                                bitboard::to_bitboard(slider_square));
        } else if (pinned == Piece::bishop || pinned == Piece::queen) {
          uint64_t moves =
              (bitboard::between_diagonal(king_square, slider_square) |
               bitboard::to_bitboard(slider_square)) ^
              bitboard::to_bitboard(pinned_square);
          while (moves) {
            _move_list[_size++] = {pinned_square, bitboard::pop_lsb(moves),
                                   Piece::none};
          }
        }
      }
      _attacks |= slider_attacks;
    }

    sliders = board.get_piece_mask<!Stm, Piece::rook, Piece::queen>();
    while (sliders) {
      int slider_square = bitboard::pop_lsb(sliders);
      uint64_t slider_attacks =
          attacks_from<Piece::rook>(slider_square, occupancy_wo_king);
      // See if the piece checks the king.
      if (slider_attacks & bitboard::to_bitboard(king_square)) {
        _checkers |= bitboard::to_bitboard(slider_square);
      }
      // Check if the enemy slider pins a piece against the stm king.
      else if (bitboard::pop_count(
                   bitboard::between_horizonal(slider_square, king_square) &
                   occupancy_wo_king) == 1 &&
               (bitboard::between_horizonal(slider_square, king_square) &
                board.get_occupied_mask<Stm>()) != 0u) {
        int pinned_square = bitboard::get_lsb(
            bitboard::between_horizonal(slider_square, king_square) &
            occupancy_wo_king);
        Piece pinned = board.get_piece(pinned_square);
        _pinned |= bitboard::to_bitboard(pinned_square);
        if (pinned == Piece::pawn) {
          generate_pawn_pushes(
              bitboard::to_bitboard(pinned_square),
              bitboard::between_horizonal(king_square, slider_square));
        } else if (pinned == Piece::rook || pinned == Piece::queen) {
          uint64_t moves =
              (bitboard::between_horizonal(king_square, slider_square) |
               bitboard::to_bitboard(slider_square)) ^
              bitboard::to_bitboard(pinned_square);
          while (moves) {
            _move_list[_size++] = {pinned_square, bitboard::pop_lsb(moves),
                                   Piece::none};
          }
        }
      }
      _attacks |= slider_attacks;
    }

    _attacks |= pseudo_king_moves(board.get_king_square<!Stm>());

    // Undo the made moves if the king is in check.
    if (_checkers != 0u) {
      _size = 0;
    }
  }

  void push_pawn_moves(uint64_t mask, int delta) {
    // Add pawn promotions to the move list.
    uint64_t move_mask = mask & PlayerTraits<Stm>::promotion_mask;
    while (move_mask) {
      int to_square = bitboard::pop_lsb(move_mask);
      int from_square = to_square - delta;

      _move_list[_size++] = {from_square, to_square, Piece::queen};
      _move_list[_size++] = {from_square, to_square, Piece::knight};
      _move_list[_size++] = {from_square, to_square, Piece::rook};
      _move_list[_size++] = {from_square, to_square, Piece::bishop};
    }

    // Add non promotions to the move list.
    move_mask = mask & ~PlayerTraits<Stm>::promotion_mask;
    while (move_mask) {
      int square = bitboard::pop_lsb(move_mask);
      _move_list[_size++] = {square - delta, square, Piece::none};
    }
  }

  bool is_en_passant_valid(const Board &board, uint64_t valid) {
    if (board.en_passant == 0) {
      return false;
    }
    // If the en passant pawn is the checker, add the en passant square
    // to the valid mask.
    int en_passant_pawn = board.en_passant - PlayerTraits<Stm>::forward;
    if (_checkers && bitboard::get_lsb(_checkers) == en_passant_pawn) {
      valid |= bitboard::to_bitboard(board.en_passant);
    }
    // Return true if the en passant square will bitwise and with
    // the mask of valid moves.
    return (bitboard::to_bitboard(board.en_passant) & valid) != 0u;
  }

  bool en_passant_causes_check(const Board &board, int from) {
    const int captured = board.en_passant - PlayerTraits<Stm>::forward;
    const int king_square = board.get_king_square<Stm>();
    // Update the occupancy as if the move is made.
    uint64_t updated_occupancy =
        board.get_occupied_mask() ^
        bitboard::to_bitboard(from, captured, board.en_passant);
    // Generate sliders that check the king using the updated occupancy.
    uint64_t checkers =
        (attacks_from<Piece::bishop>(king_square, updated_occupancy) &
         board.get_piece_mask<!Stm, Piece::bishop, Piece::queen>()) |
        (attacks_from<Piece::rook>(king_square, updated_occupancy) &
         board.get_piece_mask<!Stm, Piece::rook, Piece::queen>());
    return checkers != 0u;
  }

  void push_en_passant(const Board &board, uint64_t valid) {
    if (is_en_passant_valid(board, valid)) {
      uint64_t attackers = pseudo_pawn_attacks(!Stm, board.en_passant) &
                           board.get_piece_mask<Stm, Piece::pawn>();
      while (attackers) {
        int from = bitboard::pop_lsb(attackers);
        if (en_passant_causes_check(board, from)) {
          continue;
        }
        _move_list[_size++] = {from, board.en_passant, Piece::none};
      }
    }
  }

  void generate_pawn_pushes(uint64_t pawns, uint64_t valid) {
    uint64_t moves = 0u;
    // Single push.
    moves = _gen.pawn_push(pawns) & valid;
    push_pawn_moves(moves, PlayerTraits<Stm>::forward);
    // Double push.
    moves = _gen.pawn_double_push(pawns) & valid;
    push_pawn_moves(moves, PlayerTraits<Stm>::forward * 2);
  }

  void generate_pawn_attacks(uint64_t pawns, uint64_t valid) {
    uint64_t moves = 0u;
    // Attacks right.
    moves = _gen.pawn_attacks_right(pawns) & valid;
    push_pawn_moves(moves, PlayerTraits<Stm>::right);
    // Attacks left.
    moves = _gen.pawn_attacks_left(pawns) & valid;
    push_pawn_moves(moves, PlayerTraits<Stm>::left);
  }

  void all_pawn_moves(const Board &board, uint64_t valid) {
    const uint64_t pawns = board.get_piece_mask<Stm, Piece::pawn>() & ~_pinned;
    generate_pawn_attacks(pawns, valid);
    generate_pawn_pushes(pawns, valid);
    push_en_passant(board, valid);
  }

  template <Piece P> void push_all(uint64_t pieces, uint64_t valid) {
    while (pieces) {
      int from_square = bitboard::pop_lsb(pieces);
      uint64_t move_mask = _gen.attacks_from<P>(from_square, valid);
      while (move_mask) {
        int to_square = bitboard::pop_lsb(move_mask);
        _move_list[_size++] = {from_square, to_square, Piece::none};
      }
    }
  }

  template <Piece P> uint64_t get_valid_piece_mask(const Board &board, Slider) {
    return board.get_piece_mask<Stm, P, Piece::queen>() & ~_pinned;
  }

  template <Piece P>
  uint64_t get_valid_piece_mask(const Board &board, NonSlider) {
    return board.get_piece_mask<Stm, P>() & ~_pinned;
  }

  template <Piece P> void push_moves(const Board &board, uint64_t valid) {
    uint64_t pieces = get_valid_piece_mask<P>(board, PieceTraits<P>::type{});
    push_all<P>(pieces, valid);
  }

  void generate_castle_moves(const Board &board) {
    int king_square = board.get_king_square<Stm>();
    if (board.can_castle_kingside<Stm>(_attacks)) {
      _move_list[_size++] = {king_square, king_square - 2, Piece::none};
    }
    if (board.can_castle_queenside<Stm>(_attacks)) {
      _move_list[_size++] = {king_square, king_square + 2, Piece::none};
    }
  }
};

#endif
