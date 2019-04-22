#include "board.h"
#include "move.h"

void Board::init()
{
    player = Player::white;
    pieces.fill({});
    occupancy.fill({});
    en_passant = 0ull;
    castle_rights = 0;
    halfmove_clock = 0;
    fullmove_number = 0;
    last_move = null_move;
}