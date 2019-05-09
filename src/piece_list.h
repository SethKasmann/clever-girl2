#ifndef PIECE_LIST_H
#define PIECE_LIST_H

#include <array>
#include <iterator>
#include <iostream>

#include "player.h"
#include "piece.h"

struct PieceList
{
    std::array<int, 10> list;
    std::array<int, 10>::iterator last;
    std::array<int, 10>::const_iterator last_const;

    PieceList()
    {
        list.fill({});
        last = list.begin();
        last_const = last;
    }

    PieceList(const PieceList& piece_list)
    {
        list = piece_list.list;
        last = std::next(list.begin(), std::distance(piece_list.begin(), piece_list.end()));
        last_const = last;
    }

    PieceList(PieceList&& piece_list) noexcept
    {
        list = piece_list.list;
        last = std::next(list.begin(), std::distance(piece_list.begin(), piece_list.end()));
        last_const = last;
    }

    std::array<int, 10>::const_iterator begin() const noexcept
    {
        return list.begin();
    }

    std::array<int, 10>::const_iterator end() const noexcept
    {
        return last;
    }

    void add(int square)
    {
        *last = square;
        ++last;
        last_const = last;
    }

    void remove(int square, int index)
    {
        --last;
        last_const = last;
        list[index] = *last;
    }
};

struct PieceListManager
{
    std::array<std::array<PieceList, 7>, 2> _piece_lists;
    std::array<int, 64> _index_map;
    PieceListManager()
    {
        _index_map.fill({});
    }

    template<Piece P>
    const PieceList& get_list(Player player) const
    {
        return _piece_lists[player][P];
    }

    void remove_piece(Player player, Piece piece, int square)
    {
        _piece_lists[player][piece].remove(square, _index_map[square]);
    }

    void add_piece(Player player, Piece piece, int square)
    {
        PieceList* list = &_piece_lists[player][piece];
        _index_map[square] = static_cast<int>(std::distance(list->begin(), list->end()));
        list->add(square);
    }

};

#endif