#ifndef PIECE_LIST_H
#define PIECE_LIST_H

#include <array>
#include <vector>
#include <iterator>
#include <iostream>

#include "player.h"
#include "piece.h"

class PieceList
{
private:
    std::array<std::array<std::vector<int>, 7>, 2> _lists;
    std::array<int, 64> _index_map;
public:
    PieceList()
    {
        _index_map.fill({});
        for (auto& arr : _lists)
        {
            for (auto& vec : arr)
            {
                vec.reserve(16);
            }
        }
    }

    /*template<Piece P>
    const std::vector<int>& get_list(Player player) const
    {
        return _piece_lists[player][P];
    }*/

    void remove_piece(Player player, Piece piece, int square)
    {
        std::vector<int>& list = _lists[player][piece];
        list[_index_map[square]] = list.back();
        list.pop_back();
    }

    void add_piece(Player player, Piece piece, int square)
    {
        _index_map[square] = static_cast<int>(_lists[player][piece].size());
        _lists[player][piece].emplace_back(square);
    }

};

#endif