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
    if (_checkers != 0u) {
      _size = 0;
      push_moves<Piece::king>(board, board.get_piece_mask<Stm, Piece::king>(),
                              ~board.get_occupied_mask<Stm>() & ~_attacks);
      if (bitboard::pop_count(_checkers) == 2) {
        return;
      }
      // if (_contact_check != 0u) {
      //    int check_square = bitboard::get_lsb(_contact_check);
      //    uint64_t from = 0ull;
      //    all_pawn_moves(board, board.get_piece_mask<Stm, Piece::pawn>() &
      //    ~_pinned, bitboard::to_bitboard(check_square));
      //    push_en_passant(board, board.get_piece_mask<Stm, Piece::pawn>() &
      //    ~_pinned, bitboard::to_bitboard(check_square),
      //    PlayerTraits<Stm>::forward); from |=
      //    pseudo_knight_moves(check_square) & board.get_piece_mask<Stm,
      //    Piece::knight>(); if (pseudo_bishop_moves(check_square) &
      //    board.get_piece_mask<Stm, Piece::bishop, Piece::queen>() & ~_pinned)
      //    {
      //        from |= attacks_from<Piece::bishop>(check_square,
      //        board.get_occupied_mask()) & board.get_piece_mask<Stm,
      //        Piece::bishop, Piece::queen>() & ~_pinned;
      //    }
      //    if (pseudo_rook_moves(check_square) & board.get_piece_mask<Stm,
      //    Piece::rook, Piece::queen>() & ~_pinned) {
      //        from |= attacks_from<Piece::rook>(check_square,
      //        board.get_occupied_mask()) & board.get_piece_mask<Stm,
      //        Piece::rook, Piece::queen>() & ~_pinned;
      //    }
      //    while (from) {
      //        int from_square = bitboard::pop_lsb(from);
      //        /*if (bitboard::to_bitboard(from_square) & _pinned) {
      //            continue;
      //        }*/
      //        _move_list[_size++] = {from_square, check_square, Piece::none};
      //    }
      //    return;
      //}
      valid_moves = _checkers | bitboard::between(board.get_king_square<Stm>(),
                                                  bitboard::get_lsb(_checkers));
    } else {
      push_moves<Piece::king>(board, board.get_piece_mask<Stm, Piece::king>(),
                              ~board.get_occupied_mask<Stm>() & ~_attacks);
      generate_castle_moves(board, _attacks);
    }
    uint64_t valid_pieces = board.get_occupied_mask<Stm>() & ~_pinned;

    // Normal Moves.
    all_pawn_moves(board,
                   board.get_piece_mask<Stm, Piece::pawn>() & valid_pieces,
                   valid_moves);
    push_en_passant(board,
                    board.get_piece_mask<Stm, Piece::pawn>() & valid_pieces,
                    valid_moves, PlayerTraits<Stm>::forward);
    push_moves<Piece::knight>(board, valid_pieces, valid_moves);
    push_moves<Piece::bishop>(board, valid_pieces, valid_moves);
    push_moves<Piece::rook>(board, valid_pieces, valid_moves);
    push_moves<Piece::queen>(board, valid_pieces, valid_moves);
  }

  size_t size() const { return _size; }

  Move get_move() {
    if (_size > 0) {
      return _move_list[--_size];
    }
    return null_move;
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
          all_pawn_moves(board, bitboard::to_bitboard(pinned_square),
                         bitboard::to_bitboard(slider_square));
          push_en_passant(board, bitboard::to_bitboard(pinned_square),
                          bitboard::to_bitboard(slider_square),
                          PlayerTraits<Stm>::forward);
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
          all_pawn_moves(
              board, bitboard::to_bitboard(pinned_square),
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
    uint64_t attacks = attacks_from<Piece::bishop>(from, board.get_occupied_mask());
    uint64_t checker = 0u;
    if (attacks & board.get_piece_mask<Stm, Piece::king>()) {
        checker = attacks & pseudo_bishop_moves(king_square) & board.get_piece_mask<!Stm, Piece::bishop, Piece::queen>();
    }
    if (checker != 0ull && !(pseudo_bishop_moves(ep_capture) &
                             checker)) {
      return true;
    } else {
      checker = 0u;
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
    const uint64_t valid_attacks = valid & board.get_occupied_mask<!Stm>();
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
  }

  template <Piece P> void push_all(int from_square, uint64_t valid, NonSlider) {
    uint64_t move_mask = pseudo_attacks_from<P>(from_square) & valid;
    while (move_mask) {
      int to_square = bitboard::pop_lsb(move_mask);
      _move_list[_size++] = {from_square, to_square, Piece::none};
    }
  }

  template <Piece P> void push_all(int from_square, uint64_t valid, Slider) {
    if ((pseudo_attacks_from<P>(from_square) & valid) != 0u) {
      uint64_t move_mask = _gen.attacks_from<P>(from_square) & valid;
      while (move_mask) {
        int to_square = bitboard::pop_lsb(move_mask);
        _move_list[_size++] = {from_square, to_square, Piece::none};
      }
    }
  }

  template <Piece P>
  void push_moves(const Board &board, uint64_t valid_piece_mask,
                  uint64_t valid) {
    uint64_t piece_mask = board.get_piece_mask<Stm, P>() & valid_piece_mask;
    while (piece_mask) {
      int from_square = bitboard::pop_lsb(piece_mask);
      push_all<P>(from_square, valid, PieceTraits<P>::type{});
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
