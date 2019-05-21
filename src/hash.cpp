#include <random>
#include <array>
#include <vector>

#include "player.h"
#include "piece.h"

struct ZobristKeys
{
    std::array<uint64_t, 2> player_keys;
    std::array<uint64_t, 16> castle_keys;
    std::array<uint64_t, 8> en_passant_keys;
    std::array<uint64_t, 896> piece_keys;
} zobrist_keys;

uint64_t& set_key(uint64_t& key, Player player)
{
    key ^= zobrist_keys.player_keys[player];
    return key;
}

uint64_t& set_key(uint64_t& key, unsigned castle_rights)
{
    key ^= zobrist_keys.castle_keys[castle_rights];
    return key;
}

uint64_t& set_key(uint64_t& key, int en_passant_square)
{
    key ^= zobrist_keys.en_passant_keys[en_passant_square % 8];
    return key;
}

uint64_t& set_key(uint64_t& key, Player player, Piece piece, int square)
{
    key ^= zobrist_keys.piece_keys[player + 2 * (piece + 7 * square)];
    return key;
}

// Initialize zobrist struct with distinct random keys.
void hash_init()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint64_t> d;

    std::vector<uint64_t> distinct_keys;
    while (distinct_keys.size() < 922)
    {
        uint64_t rand_key = d(mt);
        if (std::find(distinct_keys.begin(), distinct_keys.end(), rand_key) == distinct_keys.end())
        {
            distinct_keys.emplace_back(rand_key);
        }
    }

    int copy_idx = 0;
    for (uint64_t& key : zobrist_keys.player_keys)
    {
        key = distinct_keys[copy_idx++];
    }

    for (uint64_t& key : zobrist_keys.castle_keys)
    {
        key = distinct_keys[copy_idx++];
    }

    for (uint64_t& key : zobrist_keys.en_passant_keys)
    {
        key = distinct_keys[copy_idx++];
    }

    for (uint64_t& key : zobrist_keys.piece_keys)
    {
        key = distinct_keys[copy_idx++];
    }
}