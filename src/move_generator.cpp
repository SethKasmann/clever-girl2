#include "move_generator.h"
#include "bitboard.h"
#include "magic_moves.h"
#include "piece.h"
#include "player.h"

static uint64_t pawn_moves[2][64];
static uint64_t knight_moves[64];
static uint64_t bishop_moves[64];
static uint64_t rook_moves[64];
static uint64_t king_moves[64];

uint64_t pseudo_pawn_moves(Player player, int square) {
  return pawn_moves[player][square];
}

uint64_t pseudo_pawn_quiets(Player player, int square) {
  return pseudo_pawn_moves(player, square) & rook_moves[square];
}

uint64_t pseudo_pawn_attacks(Player player, int square) {
  return pseudo_pawn_moves(player, square) & bishop_moves[square];
}

uint64_t pseudo_knight_moves(int square) { return knight_moves[square]; }

uint64_t pseudo_bishop_moves(int square) { return bishop_moves[square]; }

uint64_t pseudo_rook_moves(int square) { return rook_moves[square]; }

uint64_t pseudo_queen_moves(int square) {
  return bishop_moves[square] | rook_moves[square];
}

uint64_t pseudo_king_moves(int square) { return king_moves[square]; }

template <Piece Stm> uint64_t slider_attacks(int square, uint64_t occupancy) {
  static_assert(Stm == Piece::bishop || Stm == Piece::rook || Stm == Piece::queen);
  switch (Stm) {
  case Piece::bishop:
    return Bmagic(static_cast<unsigned int>(square), occupancy);
  case Piece::rook:
    return Rmagic(static_cast<unsigned int>(square), occupancy);
  default:
    return Qmagic(static_cast<unsigned int>(square), occupancy);
  }
}

void move_generator_init() {
  initmagicmoves();

  // Fill the pseudo move array.
  for (int square = 0; square < 64; ++square) {
    const uint64_t mask = bitboard::get_bitboard(square);
    // Pawn moves.
    pawn_moves[Player::white][square] = (mask & ~bitboard::h_file) << 7 |
                                        (mask & ~bitboard::a_file) << 9 |
                                        mask << 8;
    pawn_moves[Player::black][square] = (mask & ~bitboard::h_file) >> 9 |
                                        (mask & ~bitboard::a_file) >> 7 |
                                        mask >> 8;
    if (mask & bitboard::rank_2) {
      pawn_moves[Player::white][square] |= mask << 16;
    }
    if (mask & bitboard::rank_7) {
      pawn_moves[Player::white][square] |= mask >> 16;
    }
    // Knight moves.
    knight_moves[square] =
        (mask & ~(bitboard::a_file | bitboard::b_file)) << 10 |
        (mask & ~(bitboard::g_file | bitboard::h_file)) >> 10 |
        (mask & ~(bitboard::a_file | bitboard::b_file)) >> 6 |
        (mask & ~(bitboard::g_file | bitboard::h_file)) << 6 |
        (mask & ~bitboard::a_file) >> 15 | (mask & ~bitboard::a_file) << 17 |
        (mask & ~bitboard::h_file) << 15 | (mask & ~bitboard::h_file) >> 17;
    // Bishop moves.
    bishop_moves[square] = Bmagic(static_cast<unsigned int>(square), 0ull);
    rook_moves[square] = Rmagic(static_cast<unsigned int>(square), 0ull);
    // King moves.
    king_moves[square] =
        (mask & ~bitboard::a_file) << 1 | (mask & ~bitboard::a_file) << 9 |
        (mask & ~bitboard::a_file) >> 7 | mask << 8 | mask >> 8 |
        (mask & ~bitboard::h_file) >> 1 | (mask & ~bitboard::h_file) << 7 |
        (mask & ~bitboard::h_file) >> 9;
  }
}
