#ifndef PIECE_H
#define PIECE_H

#include <array>

enum Piece
{
    none,
    pawn,
    knight,
    bishop,
    rook,
    queen,
    king,
    count
};

inline constexpr bool operator==(Piece first, Piece second)
{
    return static_cast<int>(first) == static_cast<int>(second);
}

inline constexpr bool operator>=(Piece first, Piece second)
{
    return static_cast<int>(first) >= static_cast<int>(second);
}

inline constexpr bool operator<=(Piece first, Piece second)
{
    return static_cast<int>(first) <= static_cast<int>(second);
}

#endif
