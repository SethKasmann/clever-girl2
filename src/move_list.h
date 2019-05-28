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

template<Player Stm>
class MoveList {
private:
  std::array<Move, 255> _move_list;
  size_t _size;
  MoveGen<Stm> _gen;
public:
  MoveList(const Board &board) : _size(0), _gen(board.get_occupied_mask()) {
    // First push king moves.
    uint64_t attacked_squares = board.get_attack_mask<!Stm>(board.get_occupied_mask() ^ board.get_piece_mask<Stm, Piece::king>());
    uint64_t valid_pieces = board.get_occupied_mask<Stm>() & ~board.pinned;
    uint64_t valid_moves = ~board.get_occupied_mask<Stm>();
    push_moves<Piece::king>(board, valid_pieces,
                            valid_moves & ~attacked_squares, PieceTraits<Piece::king>::type{});

    if ((attacked_squares & board.get_piece_mask<Stm, Piece::king>()) !=
        0ull) {
      uint64_t check_mask = get_checks(board);
      // Return in the case of double check.
      if (bitboard::pop_count(check_mask) == 2) {
        return;
      }
      valid_moves =
          check_mask | bitboard::between(board.get_king_square<Stm>(),
                                         bitboard::get_lsb(check_mask));
    } else {
      // Push castle moves and pinned pieces moves only when the side to
      // move is not in check.
      generate_castle_moves(board, attacked_squares);
      generate_pinned_piece_moves(board, valid_moves);
    }

    // Normal Moves.
    all_pawn_moves(board, board.get_piece_mask<Stm, Piece::pawn>() & valid_pieces, valid_moves);
    push_moves<Piece::knight>(board, valid_pieces, valid_moves, PieceTraits<Piece::knight>::type{});
    push_moves<Piece::bishop>(board, valid_pieces, valid_moves, PieceTraits<Piece::bishop>::type{});
    push_moves<Piece::rook>(board, valid_pieces, valid_moves, PieceTraits<Piece::rook>::type{});
    push_moves<Piece::queen>(board, valid_pieces, valid_moves, PieceTraits<Piece::queen>::type{});
  }

  size_t size() const { return _size; }

  Move get_move() {
    while (_size > 0) {
      return _move_list[--_size];
    }
    return null_move;
  }

  uint64_t get_checks(const Board &board) {
    int king_square = board.get_king_square<Stm>();
    uint64_t checkers =
        (pseudo_pawn_attacks(Stm, king_square) &
         board.get_piece_mask<!Stm, Piece::pawn>()) |
        (pseudo_knight_moves(king_square) &
         board.get_piece_mask<!Stm, Piece::knight>());
    uint64_t pseudo_attacks =
        (pseudo_rook_moves(king_square) &
         board.get_piece_mask<!Stm, Piece::rook, Piece::queen>()) |
        (pseudo_bishop_moves(king_square) &
         board.get_piece_mask<!Stm, Piece::bishop, Piece::queen>());
    while (pseudo_attacks) {
      int slider = bitboard::pop_lsb(pseudo_attacks);
      if ((bitboard::between(slider, king_square) &
           board.get_occupied_mask()) == 0ull) {
        checkers |= bitboard::to_bitboard(slider);
      }
    }
    return checkers;
  }

  void generate_pinned_piece_moves(const Board &board, uint64_t valid) {
    int king_square = board.get_king_square<Stm>();
    uint64_t pinners = board.pinners;
    while (pinners) {
      int slider_square = bitboard::pop_lsb(pinners);
      int pinned_square = bitboard::get_lsb(
          bitboard::between(king_square, slider_square) & board.pinned);
      Piece pinned = board.get_piece(pinned_square);
      bool is_diagonal = (pseudo_bishop_moves(pinned_square) &
                          bitboard::to_bitboard(slider_square)) != 0ull;

      if (pinned == Piece::pawn) {
        uint64_t moves = is_diagonal
                             ? bitboard::to_bitboard(slider_square)
                             : bitboard::between(king_square, slider_square);
        all_pawn_moves(board, bitboard::to_bitboard(pinned_square),
                       moves & valid);
      } else if ((pinned == Piece::bishop && is_diagonal) ||
                 (pinned == Piece::rook && !is_diagonal) ||
                 pinned == Piece::queen) {
        uint64_t moves = (bitboard::between(king_square, slider_square) |
                          bitboard::to_bitboard(slider_square)) &
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
    uint64_t move_mask = mask & PlayerTraits<Stm>::promotion_mask;
    while (move_mask) {
      int to_square = bitboard::pop_lsb(move_mask);
      int from_square = to_square - delta;

      _move_list[_size++] = {from_square, to_square, Piece::queen};
      _move_list[_size++] = {from_square, to_square, Piece::bishop};
      _move_list[_size++] = {from_square, to_square, Piece::rook};
      _move_list[_size++] = {from_square, to_square, Piece::knight};
    }

    // Add non promotions to the move list.
    move_mask = mask & ~PlayerTraits<Stm>::promotion_mask;
    while (move_mask) {
      int square = bitboard::pop_lsb(move_mask);
      _move_list[_size++] = {square - delta, square, Piece::none};
    }
  }

  bool en_passant_causes_check(const Board &board, int from, int ep_capture) {
    int king_square = board.get_king_square<Stm>();
    // Confirm removing the captured pawn doesn't create a slider checker
    // along a diagonal.
    uint64_t checker =
        attacks_from<Piece::bishop>(ep_capture, board.get_occupied_mask()) &
        pseudo_bishop_moves(king_square) &
        board.get_piece_mask<!Stm, Piece::bishop, Piece::queen>();
    if (checker != 0ull) {
      return true;
    }
    uint64_t new_occupancy =
        board.get_occupied_mask() ^ bitboard::to_bitboard(from);
    // Confirm removing the attacking pawn and captured pawn doesn't
    // create a slider checker along the same rank.
    if (board.on_same_row(king_square, from) &&
        (bitboard::between_horizonal(ep_capture, king_square) &
         new_occupancy) == 0ull) {
      checker = attacks_from<Piece::rook>(ep_capture, new_occupancy) &
                pseudo_rook_moves(from) &
                board.get_piece_mask<!Stm, Piece::rook, Piece::queen>();
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
    if ((bitboard::to_bitboard(captured) & valid) != 0ull) {
      uint64_t moves = pseudo_pawn_attacks(!Stm, board.en_passant) & pawns;
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
        valid & board.get_occupied_mask<!Stm>();
    const uint64_t valid_quiets = valid & board.get_empty_mask();
    uint64_t moves = 0ull;

    moves = _gen.pawn_push(pawns) & valid_quiets;
    push_pawn_moves(moves, PlayerTraits<Stm>::forward);

    moves = _gen.pawn_double_push(pawns) & valid_quiets;
    push_pawn_moves(moves, PlayerTraits<Stm>::forward * 2);

    moves = _gen.pawn_attacks_right(pawns) & valid_attacks;
    push_pawn_moves(moves, PlayerTraits<Stm>::right);

    moves = _gen.pawn_attacks_left(pawns) & valid_attacks;
    push_pawn_moves(moves, PlayerTraits<Stm>::left);

    push_en_passant(board, pawns, valid, PlayerTraits<Stm>::forward);
  }

  // Maybe just something to get the moves... ?

  template<Piece P>
  void push_moves(const Board& board, uint64_t valid_piece_mask, uint64_t valid, Slider)
  {
    uint64_t piece_mask =
        board.get_piece_mask<Stm, P>() & valid_piece_mask;
    while (piece_mask)
    {
        int from_square = bitboard::pop_lsb(piece_mask);
        if ((pseudo_attacks_from<P>(from_square) & valid) == 0u)
        {
            continue;
        }
        uint64_t move_mask = _gen.attacks_from<P>(from_square) & valid;
        while (move_mask) {
            int to_square = bitboard::pop_lsb(move_mask);
            _move_list[_size++] = { from_square, to_square, Piece::none };
        }
    }
  }

  template <Piece P>
  void push_moves(const Board &board, uint64_t valid_piece_mask,
                  uint64_t valid, NonSlider) {
    uint64_t piece_mask =
        board.get_piece_mask<Stm, P>() & valid_piece_mask;
    while (piece_mask) {
      int from_square = bitboard::pop_lsb(piece_mask);
      uint64_t move_mask = _gen.attacks_from<P>(from_square) & valid;
      while (move_mask) {
        int to_square = bitboard::pop_lsb(move_mask);
        _move_list[_size++] = {from_square, to_square, Piece::none};
      }
    }
  }

  void generate_castle_moves(const Board &board,
                             uint64_t opponent_attack_mask) {
    int king_square = board.get_king_square<Stm>();
    if (board.can_castle_kingside<Stm>(opponent_attack_mask)) {
      _move_list[_size++] = {king_square, king_square - 2, Piece::none};
    }
    if (board.can_castle_queenside<Stm>(opponent_attack_mask)) {
      _move_list[_size++] = {king_square, king_square + 2, Piece::none};
    }
  }
};

#endif