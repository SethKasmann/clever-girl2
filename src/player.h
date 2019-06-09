#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>

enum class Player
{
    white,
    black
};

template<Player P>
struct PlayerTraits;

template<>
struct PlayerTraits<Player::white>
{
    static constexpr int forward = 8;
    static constexpr int left = 9;
    static constexpr int right = 7;
    static constexpr uint64_t double_mask = 0x0000000000ff0000;
    static constexpr uint64_t promotion_mask = 0xff00000000000000;
    static uint64_t pawn_left(uint64_t pawns) noexcept
    {

    }
};

template<> struct PlayerTraits<Player::black>
{
    static constexpr int forward = -8;
    static constexpr int left = -9;
    static constexpr int right = -7;
    static constexpr uint64_t double_mask = 0x0000ff0000000000;
    static constexpr uint64_t promotion_mask = 0x00000000000000ff;
};

inline constexpr Player operator!(Player player)
{
    return static_cast<Player>(!static_cast<bool>(player));
}

inline std::ostream& operator<<(std::ostream& o, Player player)
{
    o << (player == Player::white ? "white" : "black");
    return o;
}

#endif
