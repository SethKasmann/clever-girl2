#ifndef BOARD_H
#define BOARD_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <functional>
#include <numeric>

#include "bitboard.h"
#include "move_generator.h"
#include "piece.h"
#include "player.h"
#include "move.h"

//constexpr unsigned kingside_castle_white = 1;
//constexpr unsigned queenside_castle_white = 2;
//constexpr unsigned kingside_castle_black = 4;
//constexpr unsigned queenside_castle_black = 8;

static const std::array<unsigned, 64> castle_rights_mask =
{
    14, 15, 15, 12, 15, 15, 15, 13,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    11, 15, 15, 3,  15, 15, 15, 7
};

struct Board
{
    void init();
    Player player;
    std::array<uint64_t, Piece::count> pieces;
    std::array<Piece, 64> board;
    std::array<uint64_t, 2> occupancy;
    uint64_t en_passant;
    unsigned castle_rights;
    int halfmove_clock;
    int fullmove_number;
    Move last_move;

    template<Piece... P>
    uint64_t get_piece_mask() const noexcept
    {
        static_assert(((P >= Piece::pawn && P <= Piece::king) && ...));
        return (std::get<P>(pieces) | ...);
    }

    template<Piece... P>
    uint64_t get_piece_mask(Player player) const
    {
        return occupancy[player] & get_piece_mask<P...>();
    }

    Piece get_piece(int square) const;
    uint64_t get_occupied_mask(Player player) const;
    uint64_t get_occupied_mask() const noexcept;
    uint64_t get_empty_mask() const noexcept;
    bool is_attacked(int square, Player player) const;
    bool is_attacked(int square, Player player, uint64_t occupancy) const;
    bool can_castle_kingside() const;
    bool can_castle_queenside() const;
    bool is_valid() const;
    void make_move(Move move);
    friend std::ostream& operator<<(std::ostream& o, Board board);
};

#endif
