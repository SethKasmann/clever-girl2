#include "board.h"

constexpr unsigned kingside_castle_white = 1;
constexpr unsigned queenside_castle_white = 2;
constexpr unsigned kingside_castle_black = 4;
constexpr unsigned queenside_castle_black = 8;

void Board::init()
{
    player = Player::white;
    pieces.fill({});
    occupancy.fill({});
    en_passant = 0ull;
    castle_rights = 0;
    halfmove_clock = 0;
    fullmove_number = 0;
}

Piece Board::get_piece(int square) const
{
    return board[square];
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

    if (player == Player::white)
    {
        attacks |= (get_piece_mask<Piece::pawn>(player) & ~bitboard::h_file) << 7;
        attacks |= (get_piece_mask<Piece::pawn>(player) & ~bitboard::a_file) << 9;
    }
    else
    {
        attacks |= (get_piece_mask<Piece::pawn>(player) & ~bitboard::h_file) >> 9;
        attacks |= (get_piece_mask<Piece::pawn>(player) & ~bitboard::a_file) >> 7;
    }

    for (uint64_t knights = get_piece_mask<Piece::knight>(player); knights; bitboard::pop_lsb(knights))
    {
        attacks |= MoveGenerator::generate_moves<Piece::knight>(occupancy, bitboard::get_lsb(knights), player);
    }

    for (uint64_t d_sliders = get_piece_mask<Piece::bishop, Piece::queen>(player); d_sliders; bitboard::pop_lsb(d_sliders))
    {
        attacks |= MoveGenerator::generate_moves<Piece::bishop>(occupancy, bitboard::get_lsb(d_sliders), player);
    }

    for (uint64_t h_sliders = get_piece_mask<Piece::rook, Piece::queen>(player); h_sliders; bitboard::pop_lsb(h_sliders))
    {
        attacks |= MoveGenerator::generate_moves<Piece::rook>(occupancy, bitboard::get_lsb(h_sliders), player);
    }

    attacks |= MoveGenerator::generate_moves<Piece::king>(occupancy, bitboard::get_lsb(get_piece_mask<Piece::king>(player)), player);

    return attacks;
}

bool Board::is_attacked(int square, Player player) const
{
    return is_attacked(square, player, get_occupied_mask());
}

bool Board::is_attacked(int square, Player player, uint64_t occupancy) const
{
    return (MoveGenerator::generate_moves<Piece::pawn>(occupancy, square, player) & get_piece_mask<Piece::pawn>(!player))
        | (MoveGenerator::generate_moves<Piece::knight>(occupancy, square, player) & get_piece_mask<Piece::knight>(!player))
        | (MoveGenerator::generate_moves<Piece::bishop>(occupancy, square, player) & get_piece_mask<Piece::bishop, Piece::queen>(!player))
        | (MoveGenerator::generate_moves<Piece::rook>(occupancy, square, player) & get_piece_mask<Piece::rook, Piece::queen>(!player))
        | (MoveGenerator::generate_moves<Piece::king>(occupancy, square, player) & get_piece_mask<Piece::king>(!player));
}

bool Board::can_castle_kingside() const
{
    bool can_castle = false;
    if (castle_rights & (player == Player::white ? 1 : 4))
    {
        int king_square = bitboard::get_lsb(get_piece_mask<Piece::king>(player));
        uint64_t occupancy_mask_without_king = get_occupied_mask() ^ get_piece_mask<Piece::king>(player);

        can_castle = (!(bitboard::between_horizonal(king_square, king_square - 3) & get_occupied_mask())
            && !is_attacked(king_square - 1, player, occupancy_mask_without_king)
            && !is_attacked(king_square - 2, player, occupancy_mask_without_king));
    }
    return can_castle;
}

bool Board::can_castle_queenside() const
{
    bool can_castle = false;
    if (castle_rights & (player == Player::white ? 2 : 8))
    {
        int king_square = bitboard::get_lsb(get_piece_mask<Piece::king>(player));
        uint64_t occupancy_mask_without_king = get_occupied_mask() ^ get_piece_mask<Piece::king>(player);

        can_castle = (!(bitboard::between_horizonal(king_square, king_square + 4) & get_occupied_mask())
            && !is_attacked(king_square + 1, player, occupancy_mask_without_king)
            && !is_attacked(king_square + 2, player, occupancy_mask_without_king));
    }
    return can_castle;
}

void Board::make_move(Move move)
{
    ASSERT(is_valid(), this, "Board did not pass validation.");

    Piece moved_piece = get_piece(move.from);
    Piece captured_piece = get_piece(move.to);

    // Create unmake.
    Unmake unmake{ captured_piece, false, 0ull, 0, move };

    uint64_t to_mask = bitboard::get_bitboard(move.to);
    uint64_t from_mask = bitboard::get_bitboard(move.from);

    uint64_t en_passant_mask_copy = en_passant;
    // Unmake castle mask.
    unmake.en_passant_mask = en_passant;
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
        // Set unmake en passant move.
        unmake.en_passant = true;
    }
    else if (moved_piece == Piece::pawn && abs(move.from - move.to) == 16)
    {
        int en_passant_square = move.from < move.to ? move.from + 8 : move.from - 8;
        en_passant = bitboard::get_bitboard(en_passant_square);
    }

    occupancy[player] ^= (to_mask | from_mask);
    pieces[moved_piece] ^= from_mask;
    pieces[moved_piece] |= to_mask;
    board[move.from] = Piece::none;
    board[move.to] = moved_piece;

    if (captured_piece != Piece::none)
    {
        occupancy[!player] ^= to_mask;
        if (captured_piece != moved_piece)
        {
            pieces[captured_piece] ^= to_mask;
        }
    }

    if (move.promotion != Piece::none)
    {
        board[move.to] = move.promotion;
        pieces[moved_piece] ^= to_mask;
        pieces[move.promotion] ^= to_mask;
    }

    // Set unmake castle rights.
    unmake.castle_rights = castle_rights;
    castle_rights &= castle_rights_mask[move.to];
    castle_rights &= castle_rights_mask[move.from];
    player = !player;
    unmake_stack.push(unmake);
    ASSERT(is_valid(), *this, "Board did not pass validation.");
}

void Board::unmake_move()
{
    ASSERT(is_valid(), this, "Board did not pass validation.");
    Unmake unmake = unmake_stack.top();
    Piece moved_piece = get_piece(unmake.move.to);

    player = !player;

    // Unmake castle moves.
    if (unmake.move.castle)
    {
        bool is_kingside_castle = unmake.move.from > unmake.move.to;

        int rook_from_square = is_kingside_castle ? unmake.move.to + 1 : unmake.move.to - 1;
        int rook_to_square = is_kingside_castle ? unmake.move.to - 1 : unmake.move.to + 2;

        uint64_t rook_to_mask = bitboard::get_bitboard(rook_to_square);
        uint64_t rook_from_mask = bitboard::get_bitboard(rook_from_square);

        occupancy[player] ^= (rook_to_mask | rook_from_mask);
        pieces[Piece::rook] ^= (rook_to_mask | rook_from_mask);
        board[rook_from_square] = Piece::none;
        board[rook_to_square] = Piece::rook;
    }
    else if (unmake.en_passant)
    {
        int en_passant_captured_square = unmake.move.from < unmake.move.to ? unmake.move.to - 8 : unmake.move.to + 8;
        uint64_t en_passant_captured_mask = bitboard::get_bitboard(en_passant_captured_square);
        occupancy[!player] ^= en_passant_captured_mask;
        pieces[Piece::pawn] ^= en_passant_captured_mask;
        board[en_passant_captured_square] = Piece::pawn;
    }

    uint64_t to_mask = bitboard::get_bitboard(unmake.move.to);
    uint64_t from_mask = bitboard::get_bitboard(unmake.move.from);

    occupancy[player] ^= to_mask | from_mask;
    pieces[moved_piece] ^= to_mask | from_mask;
    board[unmake.move.to] = unmake.captured;
    board[unmake.move.from] = moved_piece;

    if (unmake.captured != Piece::none)
    {
        occupancy[!player] |= to_mask;
        pieces[unmake.captured] |= to_mask;
    }

    if (unmake.move.promotion != Piece::none)
    {
        pieces[Piece::pawn] ^= from_mask;
        pieces[unmake.move.promotion] ^= from_mask;
        board[unmake.move.from] = Piece::pawn;
    }

    en_passant = unmake.en_passant_mask;
    castle_rights = unmake.castle_rights;

    unmake_stack.pop();
    ASSERT(is_valid(), this, "Board did not pass validation.");
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
    int number_of_en_passant = bitboard::pop_count(en_passant);
    if (number_of_en_passant > 1)
    {
        std::cout << ++issues << ". There are " << number_of_en_passant << " pawns.\n";
    }

    if (get_piece_mask<Piece::king>(Player::white))
    {
        int king_square = bitboard::get_lsb(get_piece_mask<Piece::king>(Player::white));
        uint64_t king_moves = MoveGenerator::generate_moves<king>(get_occupied_mask(), king_square, Player::white);
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

    int en_passant_square = board.en_passant == 0ull ? 0 : bitboard::get_lsb(board.en_passant);

    o << '\n'
        << std::setw(20) << "en_passant: " << board.en_passant << ", as square: " << en_passant_square << '\n'
        << std::setw(20) << "halfmove_clock: " << board.halfmove_clock << '\n'
        << std::setw(20) << "fullmove_number: " << board.fullmove_number << '\n'
        << std::setw(20) << "last move: " << board.unmake_stack.top().move << '>';

    return o;
}