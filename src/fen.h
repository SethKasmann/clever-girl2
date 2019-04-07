#ifndef FEN_H
#define FEN_H

#include <string>

#include "board.h"

namespace fen
{
	Board create_board(std::string fen_string);
}

#endif