#ifndef PERFT_H
#define PERFT_H

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "board.h"
#include "fen.h"
#include "move.h"
#include "move_list.h"

static constexpr int required_perft_string_size = 8;

static std::vector<std::string> speed_fen = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 7 3195901860"};

static std::vector<std::string> perft_fast_vec = {
    "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1 5 1063513",
    "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1 6 1134888",
    "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1 6 1015133",
    "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1 6 1440467",
    "5k2/8/8/8/8/8/8/4K2R w K - 0 1 6 661072",
    "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1 6 803711",
    "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1 4 1274206",
    "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1 4 1720476",
    "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1 6 3821001",
    "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1 5 1004658",
    "4k3/1P6/8/8/8/8/K7/8 w - - 0 1 6 217342",
    "8/P1k5/K7/8/8/8/8/8 w - - 0 1 6 92683",
    "K1k5/8/P7/8/8/8/8/8 w - - 0 1 6 2217",
    "8/k1P5/8/1K6/8/8/8/8 w - - 0 1 7 567584",
    "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1 4 23527",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 5 193690690",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 7 178633661",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1 5 15833292",
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 5 15833292",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 5 89941194",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 5 164075551",
    "rnb1kbnr/pp1pp1pp/1qp2p2/8/Q1P5/N7/PP1PPPPP/1RB1KBNR b Kkq - 2 4 6 490103130",
    "r3k2r/1bp2pP1/5n2/1P1Q4/1pPq4/5N2/1B1P2p1/R3K2R b KQkq c3 0 1 5 202902054",
    "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6 5 70202861"
};

static std::vector<std::string> perft_extensive_vec = {
    "rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/R1BQKBNR b KQkq - 1 1 9 2440848135252",
    "rnbqkbnr/pppppppp/8/8/8/7N/PPPPPPPP/RNBQKB1R b KQkq - 1 1 9 2470483499345",
    "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1 9 "
    "3090773583680",
    "rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/R1BQKBNR b KQkq - 1 1 9 "
    "3096505857746",
    "rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1 9 2149477156227",
    "rnbqkbnr/pppppppp/8/8/8/1P6/P1PPPPPP/RNBQKBNR b KQkq - 0 1 9 "
    "2774842822463",
    "rnbqkbnr/pppppppp/8/8/1P6/8/P1PPPPPP/RNBQKBNR b KQkq - 0 1 9 "
    "2772533545113",
    "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq - 0 1 9 2905552970419",
    "rnbqkbnr/pppppppp/8/8/8/2P5/PP1PPPPP/RNBQKBNR b KQkq - 0 1 9 "
    "3072577495123",
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq - 0 1 9 "
    "3437747391692",
    "rnbqkbnr/pppppppp/8/8/8/3P4/PPP1PPPP/RNBQKBNR b KQkq - 0 1 9 "
    "5071006040569",
    "rnbqkbnr/pppppppp/8/8/8/5P2/PPPPP1PP/RNBQKBNR b KQkq - 0 1 9 "
    "1945020011164",
    "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1 9 "
    "6459463242656",
    "rnbqkbnr/pppppppp/8/8/5P2/8/PPPPP1PP/RNBQKBNR b KQkq - 0 1 9 "
    "2418056589775",
    "rnbqkbnr/pppppppp/8/8/8/6P1/PPPPPP1P/RNBQKBNR b KQkq - 0 1 9 "
    "2853630724145",
    "rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1 9 "
    "2624128147144",
    "rnbqkbnr/pppppppp/8/8/8/7P/PPPPPPP1/RNBQKBNR b KQkq - 0 1 9 2142832044687",
    "rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq - 0 1 9 2948003834105",
    "rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1 9 "
    "7299373354878",
    "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1 9 "
    "7380003266234"};

class Clock {
private:
  std::chrono::time_point<std::chrono::steady_clock> _then;

public:
  Clock() noexcept : _then(std::chrono::steady_clock::now()) {}
  long long elapsed() const noexcept {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - _then)
        .count();
  }
};

class PerftTest {
private:
  std::string _fen;
  uint64_t _nodes_expected;
  uint64_t _nodes;
  int _depth;
  long long _milliseconds;

public:
  PerftTest()
      : _nodes_expected(0ull), _nodes(0ull), _depth(0), _milliseconds(0) {}

  PerftTest(const std::string &fen, int depth, uint64_t nodes_expected)
      : PerftTest() {
    _fen = fen;
    _depth = depth;
    _nodes_expected = nodes_expected;
  }

  const std::string &get_fen() const { return _fen; }

  int get_depth() const { return _depth; }

  uint64_t get_nodes() const { return _nodes; }

  uint64_t get_nodes_expected() const { return _nodes_expected; }

  long long get_milliseconds() const { return _milliseconds; }

  void set_nodes(uint64_t nodes) { _nodes = nodes; }

  void set_milliseconds(long long milliseconds) {
    _milliseconds = milliseconds;
  }

  bool passed() const { return _nodes_expected == _nodes; }
};

// int ep = 0;
// int captures = 0;
// int castles = 0;
// int promotions = 0;
// int checks = 0;
// int checkmates = 0;
// int double_checks = 0;

#include "move_generator.h"

template <Player Stm> inline uint64_t perft(Board &board, int depth) {
  if (depth == 0) {
    return 1ull;
  }

  MoveList<Stm> move_list(board);
  // Bulk counting.
  if (depth == 1) {
    int ret = move_list.size();
    /* Move move = move_list.get_move();
     for (; move != null_move; move = move_list.get_move()) {
       bool in_check = false;
       uint64_t occ =
           board.get_occupied_mask() ^ bitboard::to_bitboard(move.from);
       switch (board.get_piece(move.from)) {
       case Piece::pawn:
         if (board.en_passant != 0 && board.en_passant == move.to) {
           ep++;
           captures++;
           occ ^= bitboard::to_bitboard(board.en_passant +
                                        PlayerTraits<Stm>::forward);
         }
         if (pawn_attacks(Stm, move.to) &
             board.get_piece_mask<!Stm, Piece::king>()) {
           checks++;
           in_check = true;
         }
         break;
       case Piece::knight:
         if (attacks_from<Piece::knight>(move.to, occ) &
             board.get_piece_mask<!Stm, Piece::king>()) {
           checks++;
           in_check = true;
         }
         break;
       case Piece::bishop:
         if (attacks_from<Piece::bishop>(move.to, occ) &
             board.get_piece_mask<!Stm, Piece::king>()) {
           checks++;
           in_check = true;
         }
         break;
       case Piece::rook:
         if (attacks_from<Piece::rook>(move.to, occ) &
             board.get_piece_mask<!Stm, Piece::king>()) {
           checks++;
         }
         break;
       case Piece::queen:
         if (attacks_from<Piece::queen>(move.to, occ) &
             board.get_piece_mask<!Stm, Piece::king>()) {
           checks++;
           in_check = true;
         }
         break;
       case Piece::king:
         if (abs(move.from - move.to) == 2) {
           castles++;
           if (move.to > move.from) {
             if (attacks_from<Piece::rook>(move.to - 1, occ) &
                 board.get_piece_mask<!Stm, Piece::king>()) {
               checks++;
               in_check = true;
             }
           } else {
             if (attacks_from<Piece::rook>(move.to + 1, occ) &
                 board.get_piece_mask<!Stm, Piece::king>()) {
               checks++;
               in_check = true;
             }
           }
         }
         break;
       }
       int king_square = board.get_king_square<!Stm>();
       assert(move.to != king_square);
       if (attacks_from<Piece::bishop>(king_square, occ) &
           (board.get_piece_mask<Stm, Piece::bishop, Piece::queen>())) {
         if (in_check) {
           double_checks++;
         } else {
           checks++;
         }
       }
       if (attacks_from<Piece::rook>(king_square, occ) &
           (board.get_piece_mask<Stm, Piece::rook, Piece::queen>())) {
         if (in_check) {
           double_checks++;
         } else {
           checks++;
         }
       }
       if (board.get_piece(move.to) != Piece::none) {
         captures++;
       }
     }
     if (ret == 0) {
       checkmates++;
     }*/
    return ret;
  }

  uint64_t nodes = 0ull;
  Move move = move_list.get_move();
  /*if (move_list.size() == 0) {
    checkmates++;
  }*/
  for (; move != null_move; move = move_list.get_move()) {
    board.make_move<Stm>(move);
    nodes += perft<!Stm>(board, depth - 1);
    board.unmake_move<Stm>(move);
  }
  return nodes;
}

template <Player Stm> inline uint64_t perft_speed(Board &board, int depth) {
  if (depth == 0) {
    return 1ull;
  }
  MoveList<Stm> move_list(board);
  if (depth == 1) {
    return static_cast<uint64_t>(move_list.size());
  }
  uint64_t nodes = 0u;
  while (move_list.size() > 0) {
    Move move = move_list.get_move();
    board.make_move<Stm>(move);
    nodes += perft<!Stm>(board, depth - 1);
    board.unmake_move<Stm>(move);
  }
  return nodes;
}

inline void speed_test(std::vector<PerftTest> &tests) {
  uint64_t total_nodes = 0u;
  long long total_milliseconds = 0;

  std::cout << "Beginning speed test...\n";

  for (auto &test : tests) {
    Board board = fen::create_board(test.get_fen());
    std::cout << board << '\n';
    for (int depth = 1; depth <= test.get_depth(); ++depth) {
      std::cout << std::setfill('.') << std::right << std::setw(10) << "depth "
                << std::left << std::setw(4) << depth;

      Clock clock;
      uint64_t nodes = board.player == Player::white
                           ? perft<Player::white>(board, depth)
                           : perft<Player::black>(board, depth);
      long long milliseconds = clock.elapsed();
      if (milliseconds == 0) {
        std::cout << "N/A" << '\n';
      } else {
        std::cout << std::fixed << std::setprecision(2)
                  << nodes / (milliseconds / 1000.0) / 1000000 << "M n/s\n";
      }
      // Confirm even speed tests actually create the correct number
      // of nodes at the max depth.
      if (depth == test.get_depth() && nodes != test.get_nodes_expected()) {
        std::cout << "... test failed " << test.get_fen() << '\n';
      }
      total_milliseconds += clock.elapsed();
      total_nodes += nodes;
    }
  }
  std::cout << "\nResult: ";
  if (total_milliseconds == 0) {
    std::cout << "N/A\n";
  } else {
    std::cout << std::fixed << std::setprecision(2)
              << total_nodes / (total_milliseconds / 1000.0) / 1000000
              << "M n/s\n";
  }
}

inline void run_tests(std::vector<PerftTest> &perft_tests) {
  int failed = 0;
  int passed = 0;
  for (auto &perft_test : perft_tests) {
    Board board = fen::create_board(perft_test.get_fen());
    int depth = perft_test.get_depth();

    // ep = 0;
    // captures = 0;
    // castles = 0;
    // promotions = 0;
    // checks = 0;
    // checkmates = 0;
    // double_checks = 0;
    Clock clock;
    uint64_t nodes = board.player == Player::white
                         ? perft<Player::white>(board, depth)
                         : perft<Player::black>(board, depth);
    perft_test.set_milliseconds(clock.elapsed());
    // std::cout << "ep: " << ep << '\n'
    //          << "captures: " << captures << '\n'
    //          << "promotions: " << promotions << '\n'
    //          << "checks: " << checks << '\n'
    //          << "double_checks: " << double_checks << '\n'
    //          << "checkmates: " << checkmates << '\n';

    perft_test.set_nodes(nodes);

    if (!perft_test.passed()) {
      std::cout << "failed: " << perft_test.get_fen()
                << " expected: " << perft_test.get_nodes_expected()
                << " actual: " << perft_test.get_nodes() << std::endl;
      failed++;
    } else {
      passed++;
    }
    std::cout << "...passed " << passed << " ... failed " << failed
              << std::endl;
  }

  std::cout << "...finished!" << std::endl;

  uint64_t nodes = 0;
  uint64_t milliseconds = 0;

  for (const PerftTest &test : perft_tests) {
    if (test.passed()) {
      nodes += test.get_nodes();
      milliseconds += test.get_milliseconds();
    }
  }

  if (milliseconds > 0) {
    std::cout << "nps: " << std::fixed << std::setprecision(2)
              << nodes / (milliseconds / 1000.0) << std::endl;
  }
}

inline std::vector<PerftTest>
generate_tests(std::vector<std::string> perft_test_vec) {
  std::vector<PerftTest> perft_tests;

  for (auto &perft_string : perft_test_vec) {
    std::istringstream iss(perft_string);
    std::vector<std::string> perft_string_vec{
        std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>{}};
    if (perft_string_vec.size() != required_perft_string_size) {
      std::cout << "Perft string " << perft_string
                << " has incorrect length of " << perft_string_vec.size() << '.'
                << std::endl;
      continue;
    }

    // The first 6 strings become the fen string.
    std::ostringstream oss;
    std::copy(perft_string_vec.begin(),
              perft_string_vec.begin() + required_perft_string_size - 2,
              std::ostream_iterator<std::string>(oss, " "));

    // The 7th string becomes the depth.
    std::string depth_string = perft_string_vec[required_perft_string_size - 2];
    if (!std::all_of(depth_string.begin(), depth_string.end(), ::isdigit)) {
      std::cout << "Perft string " << perft_string
                << " did not contain valid int for depth." << std::endl;
      continue;
    }
    int depth = std::atoi(depth_string.c_str());

    // The 8th string becomes the number of nodes.
    std::string nodes_string = perft_string_vec[required_perft_string_size - 1];
    if (!std::all_of(nodes_string.begin(), nodes_string.end(), ::isdigit)) {
      std::cout << "Perft string " << perft_string
                << " did not contain a valid int for nodes." << std::endl;
      continue;
    }
    uint64_t nodes_expected = std::stoull(nodes_string.c_str());

    perft_tests.push_back(PerftTest(oss.str(), depth, nodes_expected));
  }

  return perft_tests;
}

inline void perft_fast() {
  std::vector<PerftTest> perft_tests = generate_tests(perft_fast_vec);
  run_tests(perft_tests);
}

inline void speed() {
  std::vector<PerftTest> speed_tests = generate_tests(speed_fen);
  speed_test(speed_tests);
}

#endif
