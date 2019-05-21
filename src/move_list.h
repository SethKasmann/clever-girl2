#ifndef MOVE_LIST_H
#define MOVE_LIST_H

#include <vector>
#include <iostream>
#include <cstdlib>

#include "board.h"
#include "bitboard.h"
#include "move.h"
#include "move_generator.h"
#include "magic_moves.h"

constexpr uint64_t promotion_mask = bitboard::rank_1 | bitboard::rank_8;

class MoveList
{
private:
    std::array<Move, 255> _move_list;
    size_t _size;
    uint64_t _check_mask;
    int _num_checkers;
    uint64_t _pin_mask;
    uint64_t _double_push_rank;
    uint64_t _valid_attack_mask;
    uint64_t _valid_quiet_mask;
    uint64_t _opponent_attack_mask;
    int _forward_delta;
    int _king_square;
public:
    MoveList(const Board& board)
        : _check_mask(0ull), _pin_mask(0ull), _double_push_rank(0ull), _valid_attack_mask(0ull), _valid_quiet_mask(0ull), _size(0), _num_checkers(0), _king_square(0)
    {
        _pin_mask = board.pinned & board.get_occupied_mask(board.player);
        _king_square = board.get_king_square(board.player);
        _opponent_attack_mask = board.get_attack_mask(!board.player, board.get_occupied_mask() ^ board.get_piece_mask<Piece::king>(board.player));
        _double_push_rank = board.player == Player::white ? bitboard::rank_3 : bitboard::rank_6;
        // The amount of shift needed for a pawn push from white's point of view.
        _forward_delta = board.player == Player::white ? 8 : -8;

        if (_opponent_attack_mask & board.get_piece_mask<Piece::king>(board.player))
        {
            set_check_mask(board);
            _num_checkers = bitboard::pop_count(_check_mask);
        }

        // Handle king moves.
        _valid_attack_mask = ~_opponent_attack_mask & board.get_occupied_mask(!board.player);
        _valid_quiet_mask = ~_opponent_attack_mask & board.get_empty_mask();
        push_moves<Piece::king>(board);

        if (_num_checkers == 0)
        {
            // Try and push castle moves if we're not in check.
            _valid_attack_mask = board.get_occupied_mask(!board.player);
            _valid_quiet_mask = board.get_empty_mask();
            generate_castle_moves(board);
            generate_pinned_piece_moves_new(board);
        }
        else if (_num_checkers == 1)
        {
            // Evasions - Reset the valid masks.
            _valid_attack_mask = _check_mask;
            _valid_quiet_mask = bitboard::between(board.get_king_square(board.player), bitboard::get_lsb(_check_mask));
        }
        else if (_num_checkers == 2)
        {
            // Double check.
            return;
        }

        // Normal Moves.
        all_pawn_moves(board);
        push_moves<Piece::knight>(board);
        push_moves<Piece::bishop>(board);
        push_moves<Piece::rook>(board);
        push_moves<Piece::queen>(board);
    }

    size_t size() const
    {
        return _size;
    }

    Move get_move()
    {
        while (_size > 0)
        {
            return _move_list[--_size];
        }
        return null_move;
    }

    void set_check_mask(const Board& board)
    {
        _check_mask = pawn_attacks(board.player, _king_square) & board.get_piece_mask<Piece::pawn>(!board.player)
            | attacks_from<Piece::knight>(_king_square, board.get_occupied_mask()) & board.get_piece_mask<Piece::knight>(!board.player)
            | attacks_from<Piece::bishop>(_king_square, board.get_occupied_mask()) & board.get_piece_mask<Piece::bishop, Piece::queen>(!board.player)
            | attacks_from<Piece::rook>(_king_square, board.get_occupied_mask()) & board.get_piece_mask<Piece::rook, Piece::queen>(!board.player);
    }

    void push_pinned_slider_moves(int pinned, int slider_attacker)
    {
        // Capture the pinned piece.
        _move_list[_size++] = { pinned, slider_attacker, Piece::none };

        // Quiet moves along the pin ray.
        uint64_t move_ray = bitboard::between(_king_square, slider_attacker) & _valid_quiet_mask;
        while (move_ray)
        {
            _move_list[_size++] = { pinned, bitboard::pop_lsb(move_ray), Piece::none };
        }
    }

    void generate_pinned_piece_moves_new(const Board& board)
    {
        uint64_t pinners = board.pinners & board.get_occupied_mask(!board.player);
        while (pinners)
        {
            int slider_square = bitboard::pop_lsb(pinners);
            int pinned_square = bitboard::get_lsb(bitboard::between(_king_square, slider_square) & board.pinned);
            Piece pinned = board.get_piece(pinned_square);

            if (bitboard::bishop_moves(pinned_square) & bitboard::get_bitboard(slider_square))
            {
                if (pinned == Piece::pawn)
                {
                    push_all_pawn_attacks(board, bitboard::get_bitboard(pinned_square), bitboard::get_bitboard(slider_square));
                }
                else if (pinned == Piece::bishop || pinned == Piece::queen)
                {
                    push_pinned_slider_moves(pinned_square, slider_square);
                }
            }
            else
            {
                if (pinned == Piece::pawn)
                {
                    push_all_pawn_moves(board, bitboard::get_bitboard(pinned_square), bitboard::between(_king_square, slider_square));
                }
                else if (pinned == Piece::rook || pinned == Piece::queen)
                {
                    push_pinned_slider_moves(pinned_square, slider_square);
                }
            }
        }
    }

    void push_pawn_moves(uint64_t mask, int delta)
    {
        // Add pawn promotions to the move list.
        uint64_t move_mask = mask & promotion_mask;
        while (move_mask)
        {
            int to_square = bitboard::pop_lsb(move_mask);
            int from_square = to_square - delta;

            _move_list[_size++] = { from_square, to_square, Piece::queen };
            _move_list[_size++] = { from_square, to_square, Piece::bishop };
            _move_list[_size++] = { from_square, to_square, Piece::rook };
            _move_list[_size++] = { from_square, to_square, Piece::knight };
        }

        // Add non promotions to the move list.
        move_mask = mask & ~promotion_mask;
        while (move_mask)
        {
            int square = bitboard::pop_lsb(move_mask);
            _move_list[_size++] = { square - delta, square, Piece::none };
        }
    }

    void push_all_pawn_moves(const Board& board, uint64_t pawn_mask, uint64_t valid_mask)
    {
        uint64_t pawn_push = 0ull;
        uint64_t pawn_dbl_push = 0ull;

        if (board.player == Player::white)
        {
            pawn_push = pawn_mask << 8 & valid_mask;
            push_pawn_moves(pawn_push, 8);
            pawn_dbl_push = (pawn_mask << 8 & board.get_empty_mask() & _double_push_rank) << 8 & valid_mask;
            push_pawn_moves(pawn_dbl_push, 16);
        }
        else
        {
            pawn_push = pawn_mask >> 8 & valid_mask;
            push_pawn_moves(pawn_push, -8);
            pawn_dbl_push = (pawn_mask >> 8 & board.get_empty_mask() & _double_push_rank) >> 8 & valid_mask;
            push_pawn_moves(pawn_dbl_push, -16);
        }
    }

    void push_all_pawn_attacks(const Board& board, uint64_t pawn_mask, uint64_t valid_mask)
    {
        uint64_t pawn_attacks_left = 0ull;
        uint64_t pawn_attacks_right = 0ull;

        if (board.player == Player::white)
        {
            pawn_attacks_left = (pawn_mask & ~bitboard::h_file) << 7 & valid_mask;
            push_pawn_moves(pawn_attacks_left, 7);
            pawn_attacks_right = (pawn_mask & ~bitboard::a_file) << 9 & valid_mask;
            push_pawn_moves(pawn_attacks_right, 9);
        }
        else
        {
            pawn_attacks_left |= (pawn_mask & ~bitboard::h_file) >> 9 & valid_mask;
            push_pawn_moves(pawn_attacks_left, -9);
            pawn_attacks_right |= (pawn_mask & ~bitboard::a_file) >> 7 & valid_mask;
            push_pawn_moves(pawn_attacks_right, -7);
        }
    }

    bool ep_pawn_can_be_captured(const Board& board)
    {
        int ep_cap_square = board.en_passant - _forward_delta;
        return !(bitboard::bishop_moves(ep_cap_square) & board.get_piece_mask<Piece::king>(board.player)
            && !(bitboard::between(_king_square, ep_cap_square) & board.get_occupied_mask())
            && attacks_from<Piece::bishop>(ep_cap_square, board.get_occupied_mask()) 
                & bitboard::bishop_moves(_king_square) 
                & board.get_piece_mask<Piece::bishop, Piece::queen>(!board.player));
    }

    bool should_check_ep(const Board&board)
    {
        bool is_ep_possible = board.en_passant && pawn_attacks(!board.player, board.en_passant) & board.get_piece_mask<Piece::pawn>(board.player);

        if (is_ep_possible)
        {
            int ep_cap_square = board.en_passant - _forward_delta;
            return (bitboard::bishop_moves(ep_cap_square) & board.get_piece_mask<Piece::king>(board.player)
                && !(bitboard::between(_king_square, ep_cap_square) & board.get_occupied_mask())
                && bitboard::bishop_moves(ep_cap_square) & bitboard::bishop_moves(_king_square) & board.get_piece_mask<Piece::bishop, Piece::queen>(!board.player))
                || (ep_cap_square / 8 == _king_square / 8
                    && bitboard::rook_moves(_king_square) & board.get_piece_mask<Piece::rook, Piece::queen>(!board.player)
                    && bitboard::pop_count(bitboard::between_horizonal(_king_square, ep_cap_square) & board.get_occupied_mask()) == 1);
        }
        return false;
    }

    void all_pawn_moves(const Board& board)
    {
        uint64_t pawns = board.get_piece_mask<Piece::pawn>(board.player) & ~_pin_mask;

        push_all_pawn_attacks(board, pawns, _valid_attack_mask);
        push_all_pawn_moves(board, pawns, _valid_quiet_mask);

        if (board.en_passant)
        {
            int ep_cap_square = board.en_passant - _forward_delta;
            if (_num_checkers == 1 && !(bitboard::get_bitboard(ep_cap_square) & _check_mask))
            {
                return;
            }
            uint64_t ep_moves = pawn_attacks(!board.player, board.en_passant) & board.get_piece_mask<Piece::pawn>(board.player) & ~_pin_mask;
            if (ep_moves && ep_pawn_can_be_captured(board))
            {
                do
                {
                    int from = bitboard::pop_lsb(ep_moves);
                    if (!(_king_square / 8 == from / 8
                        && !(bitboard::between_horizonal(ep_cap_square, _king_square) & (board.get_occupied_mask() ^ bitboard::get_bitboard(from)))
                        && attacks_from<Piece::rook>(ep_cap_square, board.get_occupied_mask() ^ bitboard::get_bitboard(from))
                            & bitboard::rook_moves(_king_square) & board.get_piece_mask<Piece::rook, Piece::queen>(!board.player)))
                    {
                        _move_list[_size++] = { from, board.en_passant, Piece::none };
                    }
                } while (ep_moves);
            }
        }
    }

    template<Piece P>
    void push_moves(const Board & board)
    {
        // Non pinned pieces only.
        uint64_t piece_mask = board.get_piece_mask<P>(board.player) & ~_pin_mask;
        while (piece_mask)
        {
            int from_square = bitboard::pop_lsb(piece_mask);
            uint64_t move_mask = attacks_from<P>(from_square, board.get_occupied_mask()) & (_valid_attack_mask | _valid_quiet_mask);
            while (move_mask)
            {
                int to_square = bitboard::pop_lsb(move_mask);
                _move_list[_size++] = { from_square, to_square, Piece::none };
            }
        }
    }

    void generate_castle_moves(const Board& board)
    {
        if (board.can_castle_kingside(_opponent_attack_mask))
        {
            _move_list[_size++] = { _king_square, _king_square - 2, Piece::none };
        }
        if (board.can_castle_queenside(_opponent_attack_mask))
        {
            _move_list[_size++] = { _king_square, _king_square + 2, Piece::none };
        }
    }

};

#endif