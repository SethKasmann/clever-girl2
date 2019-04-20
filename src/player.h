#ifndef PLAYER_H
#define PLAYER_H

enum Player
{
    white,
    black
};

inline Player operator!(Player player)
{
    return static_cast<Player>(!static_cast<bool>(player));
}

#endif
