#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include <iomanip>

struct Move
{
    int from;
    int to;
    Piece promotion;
    bool castle;
};

static const Move null_move = { 0, 0, Piece::none, 0 };

inline bool operator==(const Move& m1, const Move& m2)
{
    return m1.from == m2.from && m1.to == m2.to && m1.promotion == m2.promotion && m1.castle == m2.castle;
}

inline bool operator!=(const Move& m1, const Move& m2)
{
    return !(m1 == m2);
}

inline std::ostream& operator<<(std::ostream& o, const Move& move)
{
    o << "<Move " << &move << " from: " << move.from << " to: " << move.to << " promotion: " << move.promotion << " castle: " << std::boolalpha << move.castle << ">";
    return o;
}


#endif