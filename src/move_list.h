#ifndef MOVE_LIST_H
#define MOVE_LIST_H

#include <vector>
#include <iostream>
#include <cstdlib>
#include <functional>

#include "board.h"
#include "move.h"
#include "bitboard.h"
#include "move_generator.h"

using Generator = std::function<uint64_t(uint64_t, int)>;

constexpr uint64_t promotion_mask = bitboard::rank_1 | bitboard::rank_8;

constexpr unsigned kingside_castle_white = 1;
constexpr unsigned queenside_castle_white = 2;
constexpr unsigned kingside_castle_black = 4;
constexpr unsigned queenside_castle_black = 8;

inline bool can_castle_kingside(const Board& board)
{
    bool can_castle = false;
    if (board.castle_rights & (board.player == Player::white ? kingside_castle_white : kingside_castle_black))
    {
        int king_square = bitboard::get_lsb(board.get_piece_mask(Piece::king, board.player));
        uint64_t occupancy_mask_without_king = board.get_occupied_mask() ^ board.get_piece_mask(Piece::king, board.player);

        can_castle = (!(bitboard::between_horizonal(king_square, king_square - 3) & board.get_occupied_mask())
            && !board.is_attacked(king_square - 1, board.player, occupancy_mask_without_king)
            && !board.is_attacked(king_square - 2, board.player, occupancy_mask_without_king));
    }
    return can_castle;
}

inline bool can_castle_queenside(Board board)
{
    bool can_castle = false;
    if (board.castle_rights & (board.player == Player::white ? queenside_castle_white : queenside_castle_black))
    {
        int king_square = bitboard::get_lsb(board.get_piece_mask(Piece::king, board.player));
        uint64_t occupancy_mask_without_king = board.get_occupied_mask() ^ board.get_piece_mask(Piece::king, board.player);

        can_castle = (!(bitboard::between_horizonal(king_square, king_square + 4) & board.get_occupied_mask())
            && !board.is_attacked(king_square + 1, board.player, occupancy_mask_without_king)
            && !board.is_attacked(king_square + 2, board.player, occupancy_mask_without_king));
    }
    return can_castle;
}

class MoveList
{
private:
    std::vector<Move> _move_list;
    uint64_t _check_mask;
    int _num_checkers;
    uint64_t _pin_mask;
    uint64_t _double_push_rank;
    uint64_t _valid_attack_mask;
    uint64_t _valid_quiet_mask;
    int _forward_delta;
    Generator _generator;
public:
    MoveList(const Board& board)
        : _check_mask(0ull), _pin_mask(0ull), _double_push_rank(0ull), _valid_attack_mask(0ull), _valid_quiet_mask(0ull)
    {
        _double_push_rank = board.player == Player::white ? bitboard::rank_3 : bitboard::rank_6;
        // The amount of shift needed for a pawn push from white's point of view.
        _forward_delta = board.player == Player::white ? 8 : -8;
        // Left shift is used for white and right shift is used for black.
        _generator = board.player == white ? 
            std::function<uint64_t(uint64_t, int)>{[](uint64_t mask, int shift) { return mask << shift; }}: 
            std::function<uint64_t(uint64_t, int)>{[](uint64_t mask, int shift) { return mask >> shift; }};

        set_check_mask(board);
        _num_checkers = bitboard::pop_count(_check_mask);

        // Handle king moves.
        _valid_attack_mask = board.get_occupied_mask(!board.player);
        _valid_quiet_mask = board.get_empty_mask();
        push_moves<Piece::king>(board);

        if (_num_checkers == 0)
        {
            // Try and push castle moves if we're not in check.
            generate_castle_moves(board);
        }
        else if (_num_checkers == 1)
        {
            // Evasions - Reset the valid masks.
            _valid_attack_mask = _check_mask;
            _valid_quiet_mask = bitboard::between(bitboard::get_lsb(board.get_piece_mask(Piece::king, board.player)), bitboard::get_lsb(_check_mask));
        }
        else if (_num_checkers == 2)
        {
            // Double check.
            return;
        }

        // Make moves for pinned pieces.
        generate_pinned_piece_moves(board);

        // Normal Moves.
        generate_pawn_moves(board);
        push_moves<Piece::knight>(board);
        push_moves<Piece::bishop>(board);
        push_moves<Piece::rook>(board);
        push_moves<Piece::queen>(board);
    }

    size_t size() const
    {
        return _move_list.size();
    }

    Move get_move()
    {
        while (_move_list.size() > 0)
        {
            Move next_move = _move_list.back();
            _move_list.pop_back();
            return next_move;
        }
        return null_move;
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

    void generate_pinned_piece_moves(const Board& board)
    {
        int king_square = bitboard::get_lsb(board.get_piece_mask(Piece::king, board.player));

        uint64_t diagonal_slider_mask = board.get_piece_mask(Piece::bishop, !board.player) | board.get_piece_mask(Piece::queen, !board.player);
        for (; diagonal_slider_mask; bitboard::pop_lsb(diagonal_slider_mask))
        {
            int attacker_square = bitboard::get_lsb(diagonal_slider_mask);
            uint64_t pin_ray = bitboard::between_diagonal(king_square, attacker_square);
            uint64_t possible_pin_mask = pin_ray & board.get_occupied_mask();
            if (possible_pin_mask & board.get_occupied_mask(board.player) && bitboard::pop_count(possible_pin_mask) == 1)
            {
                _pin_mask |= possible_pin_mask;
                int pin_square = bitboard::get_lsb(possible_pin_mask);
                // Pawns can only attack a diagonal slider.
                if (board.get_piece(pin_square) == Piece::pawn)
                {
                    push_all_pawn_attacks(possible_pin_mask, bitboard::get_bitboard(attacker_square) & _valid_attack_mask);
                }
                else if (board.get_piece(pin_square) == Piece::bishop || board.get_piece(pin_square) == Piece::queen)
                {
                    // Bishop/Queen attacks opponents diagonal slider.
                    if (bitboard::get_bitboard(attacker_square) & _valid_attack_mask)
                    {
                        _move_list.push_back({ pin_square, attacker_square, Piece::none, false });
                    }
                    // Bishop/Queen quiet moves on the pin ray.
                    for (uint64_t valid_quiet_mask = pin_ray & _valid_quiet_mask; valid_quiet_mask; bitboard::pop_lsb(valid_quiet_mask))
                    {
                        int to_square = bitboard::get_lsb(valid_quiet_mask);
                        _move_list.push_back({ pin_square, to_square, Piece::none, false });
                    }
                }
            }
        }

        uint64_t horizontal_slider_mask = board.get_piece_mask(Piece::rook, !board.player) | board.get_piece_mask(Piece::queen, !board.player);
        for (; horizontal_slider_mask; bitboard::pop_lsb(horizontal_slider_mask))
        {
            int attacker_square = bitboard::get_lsb(horizontal_slider_mask);
            uint64_t pin_ray = bitboard::between_horizonal(king_square, attacker_square);
            uint64_t possible_pin_mask = pin_ray & board.get_occupied_mask();
            if (possible_pin_mask & board.get_occupied_mask(board.player) && bitboard::pop_count(possible_pin_mask) == 1)
            {
                _pin_mask |= possible_pin_mask;
                int pin_square = bitboard::get_lsb(possible_pin_mask);
                // Pawns can only push a horizonal slider.
                if (board.get_piece(pin_square) == Piece::pawn)
                {
                    push_all_pawn_moves(possible_pin_mask, pin_ray & _valid_quiet_mask, board.get_empty_mask());
                }
                else if (board.get_piece(pin_square) == Piece::rook || board.get_piece(pin_square) == Piece::queen)
                {
                    // Rook/Queen attacks opponents diagonal slider.
                    if (bitboard::get_bitboard(attacker_square) & _valid_attack_mask)
                    {
                        _move_list.push_back({ pin_square, attacker_square, Piece::none, false });
                    }
                    // Rook/Queen quiet moves on the pin ray.
                    for (uint64_t valid_quiet_mask = pin_ray & _valid_quiet_mask; valid_quiet_mask; bitboard::pop_lsb(valid_quiet_mask))
                    {
                        int to_square = bitboard::get_lsb(valid_quiet_mask);
                        _move_list.push_back({ pin_square, to_square, Piece::none, false });
                    }
                }
            }
        }
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

    uint64_t move_pawns(uint64_t from_mask, uint64_t to_mask, int delta)
    {
        return _generator(from_mask, std::abs(delta)) & to_mask;
    }

    void push_all_pawn_attacks(uint64_t pawn_mask, uint64_t valid_mask)
    {
        // Pawn attacks left.
        uint64_t pawn_left_attack_mask = move_pawns(pawn_mask & ~bitboard::a_file, valid_mask, _forward_delta + 1);
        push_pawn_moves(pawn_left_attack_mask, _forward_delta + 1);

        // Pawn attacks right.
        uint64_t pawn_right_attack_mask = move_pawns(pawn_mask & ~bitboard::h_file, valid_mask, _forward_delta - 1);
        push_pawn_moves(pawn_right_attack_mask, _forward_delta - 1);
    }

    void push_all_pawn_moves(uint64_t pawn_mask, uint64_t valid_mask, uint64_t empty_mask)
    {
        // Single pawn push.
        uint64_t single_push_mask = move_pawns(pawn_mask, valid_mask, _forward_delta);
        push_pawn_moves(single_push_mask, _forward_delta);

        // Double pawn push.
        uint64_t double_push_mask = move_pawns(move_pawns(pawn_mask, empty_mask, _forward_delta) & _double_push_rank, valid_mask, _forward_delta);
        push_pawn_moves(double_push_mask, _forward_delta * 2);
    }

    void generate_pawn_moves(const Board& board)
    {
        push_all_pawn_attacks(board.get_piece_mask(Piece::pawn, board.player) & ~_pin_mask, _valid_attack_mask | board.en_passant);

        // Confirm en passant moves don't leave the king in check.
        if (board.en_passant && 
            Pmagic(bitboard::get_lsb(board.en_passant), board.get_piece_mask(Piece::pawn, board.player), !board.player))
        {
            int en_passant_square = bitboard::get_lsb(board.en_passant);
            int king_square = bitboard::get_lsb(board.get_piece_mask(Piece::king, board.player));
            int delta = _forward_delta;
            // Iterate the move list and confirm the en passant move was legal.
            //for (int i = 0; i < _move_list.size(); ++i)
            //{
            //    Move move = _move_list[i];
            //    // Confirm the move is en passant, the king square is on the same rank as the from square and the enemy pawn being removed.
            //    if (move.to == en_passant_square && board.get_piece(move.from) == Piece::pawn)
            //    {
            //        std::cout << move << std::endl;
            //        // Adjust the occupany as if the en passant move is made.
            //        uint64_t adjusted_occupancy =
            //            board.get_occupied_mask() ^ (bitboard::get_bitboard(move.from) | bitboard::get_bitboard(move.to - delta) | board.en_passant);
            //        bitboard::pretty(adjusted_occupancy);
            //        if (board.is_attacked(king_square, board.player, adjusted_occupancy))
            //        {
            //            _move_list[i] = _move_list.back();
            //            _move_list.pop_back();
            //        }
            //    }
            //}
            auto end = std::remove_if(_move_list.begin(), _move_list.end(), [&](Move move) {
                // Confirm the move is en passant, the king square is on the same rank as the from square and the enemy pawn being removed.
                if (move.to == en_passant_square && board.get_piece(move.from) == Piece::pawn)
                {
                    // Adjust the occupany as if the en passant move is made.
                    uint64_t adjusted_occupancy = 
                        board.get_occupied_mask() ^ (bitboard::get_bitboard(move.from) | bitboard::get_bitboard(move.to - delta) | board.en_passant);
                    return board.is_attacked(king_square, board.player, adjusted_occupancy);
                }
                return false;
            });
            _move_list.erase(end, _move_list.end());
        }

        push_all_pawn_moves(board.get_piece_mask(Piece::pawn, board.player) & ~_pin_mask, _valid_quiet_mask, board.get_empty_mask());
    }

    template<Piece P>
    void push_moves(const Board & board)
    {
        // Non pinned pieces only.
        uint64_t piece_mask = board.get_piece_mask(P, board.player) & ~_pin_mask;
        for (; piece_mask; bitboard::pop_lsb(piece_mask))
        {
            int from_square = bitboard::get_lsb(piece_mask);
            uint64_t move_mask = MoveGenerator::generate_moves<P>(board.get_occupied_mask(), from_square) & (_valid_attack_mask | _valid_quiet_mask);
            for (; move_mask; bitboard::pop_lsb(move_mask))
            {
                int to_square = bitboard::get_lsb(move_mask);
                if (P == Piece::king)
                {
                    // Confirm the king isn't moving into check.
                    if (board.is_attacked(to_square, board.player, board.get_occupied_mask() ^ piece_mask))
                    {
                        continue;
                    }
                }
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

};

#endif