#ifndef PERFT_H
#define PERFT_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <chrono>

#include "fen.h"
#include "board.h"
#include "move.h"
#include "move_list.h"

static constexpr int required_perft_string_size = 8;

static std::vector<std::string> perft_fast_vec =
{
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
    "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1 4 23527"
};

static std::vector<std::string> perft_extensive_vec =
{
    "rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/R1BQKBNR b KQkq - 1 1 9 2440848135252",
    "rnbqkbnr/pppppppp/8/8/8/7N/PPPPPPPP/RNBQKB1R b KQkq - 1 1 9 2470483499345",
    "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1 9 3090773583680",
    "rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/R1BQKBNR b KQkq - 1 1 9 3096505857746",
    "rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1 9 2149477156227",
    "rnbqkbnr/pppppppp/8/8/8/1P6/P1PPPPPP/RNBQKBNR b KQkq - 0 1 9 2774842822463",
    "rnbqkbnr/pppppppp/8/8/1P6/8/P1PPPPPP/RNBQKBNR b KQkq - 0 1 9 2772533545113",
    "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq - 0 1 9 2905552970419",
    "rnbqkbnr/pppppppp/8/8/8/2P5/PP1PPPPP/RNBQKBNR b KQkq - 0 1 9 3072577495123",
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq - 0 1 9 3437747391692",
    "rnbqkbnr/pppppppp/8/8/8/3P4/PPP1PPPP/RNBQKBNR b KQkq - 0 1 9 5071006040569",
    "rnbqkbnr/pppppppp/8/8/8/5P2/PPPPP1PP/RNBQKBNR b KQkq - 0 1 9 1945020011164",
    "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1 9 6459463242656",
    "rnbqkbnr/pppppppp/8/8/5P2/8/PPPPP1PP/RNBQKBNR b KQkq - 0 1 9 2418056589775",
    "rnbqkbnr/pppppppp/8/8/8/6P1/PPPPPP1P/RNBQKBNR b KQkq - 0 1 9 2853630724145",
    "rnbqkbnr/pppppppp/8/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1 9 2624128147144",
    "rnbqkbnr/pppppppp/8/8/8/7P/PPPPPPP1/RNBQKBNR b KQkq - 0 1 9 2142832044687",
    "rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq - 0 1 9 2948003834105",
    "rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1 9 7299373354878",
    "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1 9 7380003266234"
};

class PerftTest
{
private:
    std::string _fen;
    uint64_t _nodes_expected;
    uint64_t _nodes;
    int _depth;
    long long _milliseconds;
public:
    PerftTest()
        : _nodes_expected(0ull), _nodes(0ull), _depth(0), _milliseconds(0)
    { }

    PerftTest(const std::string& fen, int depth, uint64_t nodes_expected)
        : PerftTest()
    {
        _fen = fen;
        _depth = depth;
        _nodes_expected = nodes_expected;
    }

    const std::string& get_fen() const
    {
        return _fen;
    }

    int get_depth() const
    {
        return _depth;
    }

    uint64_t get_nodes() const
    {
        return _nodes;
    }

    uint64_t get_nodes_expected() const
    {
        return _nodes_expected;
    }

    long long get_milliseconds() const
    {
        return _milliseconds;
    }

    void set_nodes(uint64_t nodes)
    {
        _nodes = nodes;
    }

    void set_milliseconds(long long milliseconds)
    {
        _milliseconds = milliseconds;
    }

    bool passed() const
    {
        return _nodes_expected == _nodes;
    }
};

inline uint64_t perft(Board& board, int depth)
{
    if (depth == 0)
    {
        return 1ull;
    }

    MoveList move_list(board);
    // Bulk counting.
    if (depth == 1)
    {
        return move_list.size();
    }

    uint64_t nodes = 0ull;
    Move move = move_list.get_move();
    for (; move != null_move; move = move_list.get_move())
    {
        //Board copy = board;
        board.make_move(move);
        nodes += perft(board, depth - 1);
        board.unmake_move();
    }
    return nodes;
}

inline void run_tests(std::vector<PerftTest>& perft_tests)
{
    int failed = 0;
    int passed = 0;
    for (auto& perft_test : perft_tests)
    {
        Board board = fen::create_board(perft_test.get_fen());
        int depth = perft_test.get_depth();

        std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

        uint64_t nodes = perft(board, depth);
        perft_test.set_nodes(nodes);

        std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();

        long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        perft_test.set_milliseconds(milliseconds);

        if (!perft_test.passed())
        {
            std::cout << "failed: " << perft_test.get_fen()
                << " expected: " << perft_test.get_nodes_expected()
                << " actual: " << perft_test.get_nodes() << std::endl;
            failed++;
        }
        else 
        {
            passed++;
        }
        std::cout << "...passed " << passed << " ... failed " << failed << std::endl;
    }

    std::cout << "...finished!" << std::endl;

    uint64_t nodes = 0;
    uint64_t milliseconds = 0;

    for (const PerftTest& test : perft_tests)
    {
        if (test.passed())
        {
            nodes += test.get_nodes();
            milliseconds += test.get_milliseconds();
        }
    }

    if (milliseconds > 0)
    {
        std::cout << "nps: " << std::fixed << std::setprecision(2) 
            << nodes / (milliseconds / 1000.0) << std::endl;
    }
   
}

inline std::vector<PerftTest> generate_tests(std::vector<std::string> perft_test_vec)
{
    std::vector<PerftTest> perft_tests;

    for (auto& perft_string : perft_test_vec)
    {
        std::istringstream iss(perft_string);
        std::vector<std::string> perft_string_vec{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
        if (perft_string_vec.size() != required_perft_string_size)
        {
            std::cout << "Perft string " << perft_string << " has incorrect length of " << perft_string_vec.size() << '.' << std::endl;
            continue;
        }

        // The first 6 strings become the fen string.
        std::ostringstream oss;
        std::copy(perft_string_vec.begin(), perft_string_vec.begin() + required_perft_string_size - 2,
            std::ostream_iterator<std::string>(oss, " "));

        // The 7th string becomes the depth. 
        std::string depth_string = perft_string_vec[required_perft_string_size - 2];
        if (!std::all_of(depth_string.begin(), depth_string.end(), ::isdigit))
        {
            std::cout << "Perft string " << perft_string << " did not contain valid int for depth." << std::endl;
            continue;
        }
        int depth = std::atoi(depth_string.c_str());

        // The 8th string becomes the number of nodes.
        std::string nodes_string = perft_string_vec[required_perft_string_size - 1];
        if (!std::all_of(nodes_string.begin(), nodes_string.end(), ::isdigit))
        {
            std::cout << "Perft string " << perft_string << " did not contain a valid int for nodes." << std::endl;
            continue;
        }
        uint64_t nodes_expected = std::atoi(nodes_string.c_str());

        perft_tests.push_back(PerftTest(oss.str(), depth, nodes_expected));
    }

    return perft_tests;
}

inline void perft_fast()
{
    std::vector<PerftTest> perft_tests = generate_tests(perft_fast_vec);
    run_tests(perft_tests);
}



#endif
