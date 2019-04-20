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
#include "magic_moves.h"
#include "piece.h"
#include "player.h"
#include "move.h"

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

    Piece get_piece(int square) const
    {
        return board[square];
    }

    uint64_t get_piece_mask(Piece piece, Player player) const
    {
        return pieces[piece] & occupancy[player];
    }

    uint64_t get_occupied_mask(Player player) const
    {
        return occupancy[player];
    }

    uint64_t get_occupied_mask() const
    {
        return occupancy[Player::white] | occupancy[Player::black];
    }

    uint64_t get_empty_mask() const
    {
        return ~(get_occupied_mask());
    }

    bool is_valid() const
    {
        int issues = 0;
        for (int i = 0; i < 64; ++i)
        {
            uint64_t bit_mask = bitboard::get_bitboard(i);
            if (occupancy[Player::white] & bit_mask && occupancy[Player::black] & bit_mask)
            {
                std::cout << ++issues << ". Occupancy mask is set for both players at square " << i << ".\n";
            }

            Piece piece_on_board = board[i];
            if (piece_on_board == Piece::none)
            {
                if (std::accumulate(pieces.begin() + 1, pieces.end(), 0ull, std::bit_or<uint64_t>()) & bit_mask)
                {
                    std::cout << ++issues << ". Board is empty while pieces contains a set bit at square " << i << ".\n";
                }
                if (get_occupied_mask() & bit_mask)
                {
                    std::cout << ++issues << ". Board is empty while occupancy contains a set bit at square " << i << ".\n";
                }
            }
            else
            {
                if (!(pieces[piece_on_board] & bit_mask))
                {
                    std::cout << ++issues << ". Board piece " << piece_on_board << " is not on piece mask at square " << i << ".\n";
                }
                if (!(get_occupied_mask() & bit_mask))
                {
                    std::cout << ++issues << ". Board piece " << piece_on_board << " is not on occupancy mask at square " << i << ".\n";
                }
                for (int piece = Piece::pawn; piece <= Piece::king; ++piece)
                {
                    if (piece_on_board != static_cast<Piece>(piece) && pieces[piece] & bit_mask)
                    {
                        std::cout << ++issues << ". Board piece " << piece_on_board << " is on " << piece << " mask at " << i << ".\n";
                    }
                }
            }
        }
        int number_of_kings = bitboard::pop_count(pieces[Piece::king]);
        if (number_of_kings != 2)
        {
            std::cout << ++issues << ". The number of kings is not valid at " << number_of_kings << ".\n";
        }
        return issues == 0;
    }

    void make_move(Move move)
    {
        ASSERT(is_valid(), this, "Board did not pass validation.");

        Piece moved_piece = get_piece(move.from);
        Piece captured_piece = get_piece(move.to);

        uint64_t to_mask = bitboard::get_bitboard(move.to);
        uint64_t from_mask = bitboard::get_bitboard(move.from);

        uint64_t en_passant_mask_copy = en_passant;
        en_passant = 0ull;

        if (move.castle)
        {
            bool is_kingside_castle = move.from > move.to;

            int rook_to_square = is_kingside_castle ? move.to + 1 : move.to - 1;
            int rook_from_square = is_kingside_castle ? move.to - 1 : move.to + 2;

            uint64_t rook_to_mask = bitboard::get_bitboard(rook_to_square);
            uint64_t rook_from_mask = bitboard::get_bitboard(rook_from_square);

            occupancy[player] ^= (rook_to_mask | rook_from_mask);
            pieces[Piece::rook] ^= (rook_to_mask | rook_from_mask);
            board[rook_from_square] = Piece::none;
            board[rook_to_square] = Piece::rook;
        }
        else if (moved_piece == Piece::pawn && to_mask == en_passant_mask_copy)
        {
            int en_passant_captured_square = move.from < move.to ? move.to - 8 : move.to + 8;
            uint64_t en_passant_captured_mask = bitboard::get_bitboard(en_passant_captured_square);
            occupancy[!player] ^= en_passant_captured_mask;
            pieces[Piece::pawn] ^= en_passant_captured_mask;
            board[en_passant_captured_square] = Piece::none;
        }
        else if (moved_piece == Piece::pawn && abs(move.from - move.to) == 16)
        {
            int en_passant_square = move.from < move.to ? move.from + 8 : move.from - 8;
            en_passant = bitboard::get_bitboard(en_passant_square);
        }

        occupancy[player] ^= (to_mask | from_mask);
        pieces[moved_piece] ^= (to_mask | from_mask);
        board[move.from] = Piece::none;
        board[move.to] = moved_piece;

        if (captured_piece != Piece::none && captured_piece != moved_piece)
        {
            occupancy[!player] ^= to_mask;
            pieces[captured_piece] ^= to_mask;
        }

        castle_rights &= castle_rights_mask[move.to];
        castle_rights &= castle_rights_mask[move.from];
        ASSERT(is_valid(), this, "Board did not pass validation.");
    }
};

inline std::ostream& operator<<(std::ostream& o, const Board& board)
{
    const std::array<const std::array<char, 7>, 2> piece_chars
    { {
        { ' ', 'P', 'N', 'B', 'R', 'Q', 'K' },
        { ' ', 'p', 'n', 'b', 'r', 'q', 'k' }
    } };

    const std::string bar = " +---+---+---+---+---+---+---+---+";

    o << bar << std::endl;

    for (int square = 63; square >= 0; --square)
    {
        uint64_t square_bit = bitboard::get_bitboard(square);
        if (square_bit & bitboard::a_file)
        {
            o << std::to_string(bitboard::get_rank(square_bit) + 1);
        }
        o << '|' << (square_bit & board.en_passant ? '*' : ' ');
        if (square_bit & board.get_empty_mask())
        {
            o << "  ";
        }
        else
        {
            Player player = square_bit & board.occupancy[0] ? Player::white : Player::black;
            o << piece_chars[player][board.get_piece(square)] << ' ';
        }
        if (square_bit & bitboard::h_file)
        {
            o << '|' << std::endl << bar << std::endl;
        }
    }

    o << "  A   B   C   D   E   F   G   H" << std::endl;

    o << (board.player == Player::white ? "White" : "Black");
    o << " to move." << std::endl;

    o << "Castle Rights: ";
    if (board.castle_rights & 1ull)
    {
        o << "K";
    }
    if (board.castle_rights & 1ull << 1)
    {
        o << "Q";
    }
    if (board.castle_rights & 1ull << 2)
    {
        o << "k";
    }
    if (board.castle_rights & 1ull << 3)
    {
        o << "q";
    }
    o << std::endl;

    o << "Last Move: " << board.last_move << std::endl;

    return o;
}

#endif
