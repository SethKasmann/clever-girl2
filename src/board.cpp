#include "board.h"

void Board::init()
{
	wtm = true;
	pieces.fill({});
	occupancy.fill({});
	en_passant = 0ull;
	castle_rights = 0;
	halfmove_clock = 0;
	fullmove_number = 0;
}