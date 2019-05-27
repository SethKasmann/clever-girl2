#include "board.h"
#include "hash.h"
#include "move_generator.h"

constexpr unsigned kingside_castle_white = 1;
constexpr unsigned queenside_castle_white = 2;
constexpr unsigned kingside_castle_black = 4;
constexpr unsigned queenside_castle_black = 8;

Board::Board()
{
    player = Player::white;
    pieces.fill({});
    occupancy.fill({});
    board.fill({});
    en_passant = 0;
    castle_rights = 0;
    halfmove_clock = 0;
    fullmove_number = 0;
    key = 0ull;
    unmake_stack.reserve(255);
    pinned = 0ull;
    pinners = 0ull;
}

void Board::init()
{
    set_pins(player);
}

Piece Board::get_piece(int square) const
{
    return board[square];
}

void Board::put_piece(Player player, Piece piece, int square)
{
    uint64_t square_bit = bitboard::get_bitboard(square);
    occupancy[player] |= square_bit;
    pieces[piece] |= square_bit;
    board[square] = piece;
    set_key(key, player, piece, square);
}

void Board::remove_piece(Player player, int square)
{
    uint64_t square_bit = bitboard::get_bitboard(square);
    Piece piece = board[square];
    occupancy[player] ^= square_bit;
    pieces[piece] ^= square_bit;
    board[square] = Piece::none;
    set_key(key, player, piece, square);
}

uint64_t Board::get_occupied_mask(Player player) const
{
    return occupancy[player];
}

uint64_t Board::get_occupied_mask() const noexcept
{
    return std::get<Player::white>(occupancy) | std::get<Player::black>(occupancy);
}

uint64_t Board::get_empty_mask() const noexcept
{
    return ~(get_occupied_mask());
}

uint64_t Board::get_attack_mask(Player player, uint64_t occupancy) const
{
    uint64_t attacks = 0ull;

    attacks |= pawn_attacks(player, get_piece_mask<Piece::pawn>(player));

    uint64_t knights = get_piece_mask<Piece::knight>(player);
    while (knights)
    {
        attacks |= attacks_from<Piece::knight>(bitboard::pop_lsb(knights), occupancy);
    }

    uint64_t d_sliders = get_piece_mask<Piece::bishop, Piece::queen>(player);
    while (d_sliders)
    {
        attacks |= attacks_from<Piece::bishop>(bitboard::pop_lsb(d_sliders), occupancy);
    }

    uint64_t h_sliders = get_piece_mask<Piece::rook, Piece::queen>(player);
    while (h_sliders)
    {
        attacks |= attacks_from<Piece::rook>(bitboard::pop_lsb(h_sliders), occupancy);
    }

    attacks |= pseudo_king_moves(get_king_square(player));

    return attacks;
}

bool Board::can_castle_kingside(uint64_t attack_mask) const
{
    bool can_castle = false;
    if (castle_rights & (player == Player::white ? kingside_castle_white : kingside_castle_black))
    {
        int king_square = get_king_square(player);

        // Confirm there are no pieces blocking the king from castling and the king does not pass
        // through an attacked square.
        can_castle = (!(bitboard::between_horizonal(king_square, king_square - 3) & get_occupied_mask())
            && !(bitboard::between_horizonal(king_square, king_square - 3) & attack_mask));
    }
    return can_castle;
}

bool Board::can_castle_queenside(uint64_t attack_mask) const
{
    bool can_castle = false;
    if (castle_rights & (player == Player::white ? queenside_castle_white : queenside_castle_black))
    {
        int king_square = get_king_square(player);

        // Confirm there are no pieces blocking the king from castling and the king does not pass
        // through an attacked square.
        can_castle = (!(bitboard::between_horizonal(king_square, king_square + 4) & get_occupied_mask())
            && !(bitboard::between_horizonal(king_square, king_square + 3) & attack_mask));
    }
    return can_castle;
}

void Board::make_move(Move move)
{
    ASSERT(is_valid(), this, "Board did not pass validation.");

    const Piece moved = get_piece(move.from);
    const Piece captured = get_piece(move.to);

    unmake_stack.emplace_back(Unmake{captured, en_passant, castle_rights, key, pinned, pinners});

    // Update piece location.
    if (captured != Piece::none)
    {
        remove_piece(!player, move.to);
    }
    remove_piece(player, move.from);
    put_piece(player, moved, move.to);

    if (en_passant)
    {
        // Check if the move captures a pawn en passant.
        if (move.to == en_passant && moved == Piece::pawn)
        {
            int capture_square = en_passant + (move.from < move.to ? -8 : 8);
            remove_piece(!player, capture_square);
        }
        // Clear en passant square.
        set_key(key, en_passant);
        en_passant = 0;
    }

    // Check for double push.
    if (moved == Piece::pawn && abs(move.from - move.to) == 16)
    {
        // Set new en passant square.
        en_passant = (move.from + move.to) / 2;
        set_key(key, en_passant);
    }

    // Check for castling moves.
    if (moved == Piece::king && abs(move.from - move.to) == 2)
    {
        if (move.from > move.to) // Kingside castle
        {
            remove_piece(player, move.to - 1);
            put_piece(player, Piece::rook, move.to + 1);
        }
        else // Queenside castle
        {
            remove_piece(player, move.to + 2);
            put_piece(player, Piece::rook, move.to - 1);
        }
    }

    // Check for promotion.
    if (move.promotion != Piece::none)
    {
        remove_piece(player, move.to);
        put_piece(player, move.promotion, move.to);
    }
    
    // Update castle rights.
    if (castle_rights)
    {
        set_key(key, castle_rights);
        castle_rights &= castle_rights_mask[move.to];
        castle_rights &= castle_rights_mask[move.from];
        set_key(key, castle_rights);
    }

    // Update the side to move.
    set_key(key, player);
    player = !player;
    set_key(key, player);

    pinners = 0ull;
    pinned = 0ull;
    set_pins(player);

    ASSERT(is_valid(), *this, "Board did not pass validation.");
}

void Board::unmake_move(Move move)
{
    ASSERT(is_valid(), this, "Board did not pass validation.");

    Unmake unmake = unmake_stack.back();

    const Piece moved = get_piece(move.to);
    const Piece captured = unmake.captured;

    // Update side to move.
    set_key(key, player);
    player = !player;
    set_key(key, player);

    // Update piece location.
    remove_piece(player, move.to);
    put_piece(player, moved, move.from);
    if (captured != Piece::none)
    {
        put_piece(!player, captured, move.to);
    }

    // Clear en passant.
    if (en_passant)
    {
        set_key(key, en_passant);
    }

    en_passant = unmake.en_passant;
    if (en_passant)
    {
        // Check if the move captured a pawn en passant.
        if (move.to == en_passant && moved == Piece::pawn)
        {
            int captured_square = en_passant + (move.from < move.to ? -8 : 8);
            put_piece(!player, Piece::pawn, captured_square);
        }
        set_key(key, en_passant);
    }

    // Check for castling moves.
    if (moved == Piece::king && abs(move.from - move.to) == 2)
    {
        if (move.from > move.to) // Kingside castle
        {
            remove_piece(player, move.to + 1);
            put_piece(player, Piece::rook, move.to - 1);
        }
        else // Queenside castle
        {
            remove_piece(player, move.to - 1);
            put_piece(player, Piece::rook, move.to + 2);
        }
    }

    // Check for promotion.
    if (move.promotion != Piece::none)
    {
        remove_piece(player, move.from);
        put_piece(player, Piece::pawn, move.from);
    }

    // Update castle rights.
    if (unmake.castle_rights)
    {
        set_key(key, castle_rights);
        castle_rights = unmake.castle_rights;
        set_key(key, castle_rights);
    }

    pinners = unmake.pinners;
    pinned = unmake.pinned;

    ASSERT((key == unmake.key), key, "Key did not equal unmake.key");

    unmake_stack.pop_back();
    ASSERT(is_valid(), this, "Board did not pass validation.");
}

void Board::set_pins(Player player)
{
    int king_square = get_king_square(player);
    uint64_t pseudo_attacks = (pseudo_rook_moves(king_square) & get_piece_mask<Piece::rook, Piece::queen>(!player))
        | (pseudo_bishop_moves(king_square) & get_piece_mask<Piece::bishop, Piece::queen>(!player));
    while (pseudo_attacks)
    {
        int slider = bitboard::pop_lsb(pseudo_attacks);
        uint64_t between = bitboard::between(slider, king_square) & get_occupied_mask();
        if (between & get_occupied_mask(player) && bitboard::pop_count(between) == 1)
        {
            pinners |= bitboard::get_bitboard(slider);
            pinned |= between;
        }
    }
}

bool Board::is_valid() const
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
            Player current_player = static_cast<Player>(static_cast<bool>(get_occupied_mask(Player::black) & bit_mask));
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

    if (get_piece_mask<Piece::king>(Player::white))
    {
        int king_square = get_king_square(player);
        uint64_t king_moves = attacks_from<Piece::king>(king_square, get_occupied_mask());
        if (king_moves & get_piece_mask<Piece::king>(Player::black))
        {
            std::cout << ++issues << ". White and black kings are too close.\n";
        }
    }
    return issues == 0;
}

std::ostream& operator<<(std::ostream& o, Board board)
{
    const std::array<const std::array<char, 7>, 2> piece_chars
    { {
        { ' ', 'P', 'N', 'B', 'R', 'Q', 'K' },
        { ' ', 'p', 'n', 'b', 'r', 'q', 'k' }
    } };

    const std::string bar = " +---+---+---+---+---+---+---+---+";

    o << "<Board " << &board << std::endl << bar << std::endl;

    for (int square = 63; square >= 0; --square)
    {
        uint64_t square_bit = bitboard::get_bitboard(square);
        if (square_bit & bitboard::a_file)
        {
            o << std::to_string(bitboard::get_rank(square_bit) + 1);
        }
        o << '|' << (square == board.en_passant ? '*' : ' ');
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

    o << "  A   B   C   D   E   F   G   H" << "\n\n";

    for (int piece = static_cast<int>(Piece::pawn); piece <= static_cast<int>(Piece::king); ++piece)
    {
        o << std::setw(8) << std::left << piece_chars[Player::white][piece] << std::setw(4) << std::cout.fill(' ');
    }

    o << std::endl;

    for (int rank = 7; rank >= 0; --rank)
    {
        for (int piece = static_cast<int>(Piece::pawn); piece <= static_cast<int>(Piece::king); ++piece)
        {
            for (int file = 7; file >= 0; --file)
            {
                uint64_t bit = bitboard::get_bitboard(rank * 8 + file);
                std::cout << std::noboolalpha << ((board.pieces[piece] & bit) != 0ull);
            }
            std::cout << std::setw(4) << std::cout.fill(' ');
        }
        std::cout << std::endl;
    }

    o << std::endl << std::setw(20) << "player: " << board.player << '\n';

    o << std::setw(20) << "castle_rights: ";
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

    o << '\n'
        << std::setw(20) << "en_passant: " << board.en_passant << '\n'
        << std::setw(20) << "halfmove_clock: " << board.halfmove_clock << '\n'
        << std::setw(20) << "fullmove_number: " << board.fullmove_number << '>';

    return o;
}