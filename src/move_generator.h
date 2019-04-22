#ifndef MOVE_GENERATOR_H
#define MOVE_GENERATOR_H

#include <iostream>

#include "magic_moves.h"
#include "player.h"
#include "piece.h"

struct MoveGenerator
{
    template<Piece P>
    static uint64_t generate_moves(uint64_t occupancy_mask, int square, Player player = Player::white)
    {
        switch (P)
        {
        case Piece::pawn:
            return Pmagic(square, occupancy_mask, player);
        case Piece::knight:
            return Nmagic(square, occupancy_mask);
        case Piece::bishop:
            return Bmagic(square, occupancy_mask);
        case Piece::rook:
            return Rmagic(square, occupancy_mask);
        case Piece::queen:
            return Qmagic(square, occupancy_mask);
        case Piece::king:
            return Kmagic(square, occupancy_mask);
        }
    }

    static void init()
    {
        initmagicmoves();
    }
};

#endif