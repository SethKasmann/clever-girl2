#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>

enum Player
{
    white,
    black
};

inline Player operator!(Player player)
{
    return static_cast<Player>(!static_cast<bool>(player));
}

inline std::ostream& operator<<(std::ostream& o, Player player)
{
    o << (player == Player::white ? "white" : "black");
    return o;
}

#endif
