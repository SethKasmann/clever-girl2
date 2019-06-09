#ifndef BOARD_H
#define BOARD_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <functional>
#include <numeric>
#include <stack>
#include <vector>

#include "bitboard.h"
#include "piece.h"
#include "player.h"
#include "move.h"
#include "magic_moves.h"

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

struct Unmake
{
    Piece captured;
    int en_passant;
    unsigned castle_rights;
    uint64_t key;
   /* uint64_t pinned;
    uint64_t pinners;*/
};

struct Board
{
    Board();
    void init();
    Player player;
    std::array<uint64_t, Piece::count> pieces;
    std::array<Piece, 64> board;
    std::array<uint64_t, 2> occupancy;
    int en_passant;
    unsigned castle_rights;
    int halfmove_clock;
    int fullmove_number;
    std::vector<Unmake> unmake_stack;
    uint64_t key;
    /*uint64_t pinners;
    uint64_t pinned;*/

    template<Piece... P>
    constexpr uint64_t get_piece_mask() const noexcept
    {
        static_assert(((P >= Piece::pawn && P <= Piece::king) && ...));
        return (std::get<P>(pieces) | ...);
    }

    template<Player Stm, Piece... P>
    constexpr uint64_t get_piece_mask() const noexcept
    {
        static_assert(Stm == Player::white || Stm == Player::black);
        return std::get<Stm>(occupancy) & get_piece_mask<P...>();
    }

    template<Player Stm>
    int get_king_square() const
    {
        return bitboard::get_lsb(get_piece_mask<Stm, Piece::king>());
    }

    /*int get_king_square(Player player) const
    {
        return bitboard::get_lsb(get_piece_mask<Piece::king>(player));
    }*/

    Piece get_piece(int square) const;
    template<Player P>
    bool is_player(int square) const noexcept;
    void put_piece(Player player, Piece piece, int square);
    void remove_piece(Player player, int square);
    template<Player Stm>
    uint64_t get_occupied_mask() const noexcept;
    uint64_t get_occupied_mask(Player player) const;
    uint64_t get_occupied_mask() const noexcept;
    uint64_t get_empty_mask() const noexcept;
    template<Player Stm>
    uint64_t get_attack_mask(uint64_t occupancy) const;
    template<Player Stm>
    bool can_castle_kingside(uint64_t attack_mask) const;
    template<Player Stm>
    bool can_castle_queenside(uint64_t attack_mask) const;
    bool is_valid() const;
    template<Player Stm>
    void make_move(Move move);
    template<Player Stm>
    void unmake_move(Move move);
    template<Player Stm>
    void set_pins();
    static bool on_same_row(int square0, int square1)
    {
        return square0 / 8 == square1 / 8;
    }
    void pretty() const;
    friend std::ostream& operator<<(std::ostream& o, Board board);
};

#endif
