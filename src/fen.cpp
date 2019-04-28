#include <sstream>
#include <vector>
#include <unordered_map>

#include "fen.h"
#include "bitboard.h"
#include "piece.h"

namespace fen
{
    // Fullmove number: The number of the full move. It starts at 1, and is incremented after Black's move.
    void set_fullmove_number(Board& board, const std::string& fullmove_number)
    {
        if (std::all_of(fullmove_number.begin(), fullmove_number.end(), ::isdigit))
        {
            board.fullmove_number = std::atoi(fullmove_number.c_str());
        }
    }

    // Halfmove clock: This is the number of halfmoves since the last capture or pawn advance.
    void set_halfmove_clock(Board& board, const std::string& halfmove_clock)
    {
        if (std::all_of(halfmove_clock.begin(), halfmove_clock.end(), ::isdigit))
        {
            board.halfmove_clock = std::atoi(halfmove_clock.c_str());
        }
    }

    // En passant target square in algebraic notation.
    void set_en_passant(Board& board, const std::string& en_passant)
    {
        if (en_passant.size() == 2)
        {
            int file = 'h' - tolower(en_passant[0]);
            int rank = tolower(en_passant[1]) - '1';
            board.en_passant |= 1ull << (rank * 8 + file);
        }
    }

    // Castling availability. If neither side can castle, this is "-".
    void set_castling_availability(Board& board, const std::string& castling_availability)
    {
        for (auto c : castling_availability)
        {
            if (c == 'K')
            {
                board.castle_rights |= 1ull;
            }
            else if (c == 'Q')
            {
                board.castle_rights |= 1ull << 1;
            }
            else if (c == 'k')
            {
                board.castle_rights |= 1ull << 2;
            }
            else if (c == 'q')
            {
                board.castle_rights |= 1ull << 3;
            }
        }
    }

    // Active color. "w" means White moves next, "b" means Black moves next.
    void set_side_to_move(Board& board, const std::string& side_to_move)
    {
        board.player = side_to_move[0] == 'w' ? Player::white : Player::black;
    }

    // Piece placement (from White's perspective).
    void set_board_pieces(Board& board, const std::string& piece_placement)
    {
        int current_square = 63;
        const std::unordered_map<char, Piece> char_to_piece
        {
            { 'p', Piece::pawn }, { 'n', Piece::knight }, { 'b', Piece::bishop }, { 'r', Piece::rook }, { 'q', Piece::queen }, { 'k', Piece::king }
        };
        for (auto c : piece_placement)
        {
            if (c == '/') continue;
            if (isdigit(c))
            {
                int empty_squares = c - '0';
                for (int i = 0; i < empty_squares; ++i)
                {
                    board.board[current_square] = Piece::none;
                    current_square -= 1;
                }
            }
            else
            {
                Player player = isupper(c) > 0 ? Player::white : Player::black;
                Piece piece = char_to_piece.at(tolower(c));
                uint64_t square_mask = bitboard::get_bitboard(current_square);
                board.pieces[piece] |= square_mask;
                board.occupancy[player] |= square_mask;
                board.board[current_square] = piece;
                current_square -= 1;
            }
        }
    }

    // Split the fen string into a vector guaranteed to be length 6.
    std::vector<std::string> fen_str_to_vec(std::string fen_string)
    {
        std::vector<std::string> fen_vec(6);
        std::istringstream iss(fen_string);
        std::vector<std::string> fen_split{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
        std::copy(fen_split.begin(), fen_split.end(), fen_vec.begin());
        return fen_vec;
    }

    Board create_board(const std::string& fen_string)
    {
        Board board;
        board.init();
        auto fen_vec = fen_str_to_vec(fen_string);
        set_board_pieces(board, fen_vec[0]);
        set_side_to_move(board, fen_vec[1]);
        set_castling_availability(board, fen_vec[2]);
        set_en_passant(board, fen_vec[3]);
        set_halfmove_clock(board, fen_vec[4]);
        set_fullmove_number(board, fen_vec[5]);
        return board;
    }
}