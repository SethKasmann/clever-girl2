#ifndef PIECE_H
#define PIECE_H

#include <array>
#include <iostream>

enum Piece
{
    none,
    pawn,
    knight,
    bishop,
    rook,
    queen,
    king,
    count,
    all
};

struct Slider {};
struct NonSlider {};

template<Piece P>
struct PieceTraits;

template<>
struct PieceTraits<Piece::pawn>
{
    using type = NonSlider;
};

template<>
struct PieceTraits<Piece::knight>
{
    using type = NonSlider;
};

template<>
struct PieceTraits<Piece::bishop>
{
    using type = Slider;
};

template<>
struct PieceTraits<Piece::rook>
{
    using type = Slider;
};

template<>
struct PieceTraits<Piece::queen>
{
    using type = Slider;
};

template<>
struct PieceTraits<Piece::king>
{
    using type = NonSlider;
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
