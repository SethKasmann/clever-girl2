#ifndef HASH_H
#define HASH_H

#include <iostream>
#include <array>

#include "player.h"
#include "piece.h"

void hash_init();
uint64_t& set_key(uint64_t& key, Player player);
uint64_t& set_key(uint64_t& key, unsigned castle_rights);
uint64_t& set_key(uint64_t& key, int en_passant_square);
uint64_t& set_key(uint64_t& key, Player player, Piece piece, int square);

#endif
