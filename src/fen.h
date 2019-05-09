#ifndef FEN_H
#define FEN_H

#include <string>

#include "board.h"
#include "piece_list.h"

namespace fen
{
    Board create_board(const std::string& fen_string);
}

#endif