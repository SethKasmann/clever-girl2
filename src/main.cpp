#include <iostream>
#include "board.h"
#include "fen.h"
#include "move_generator.h"

int main(int argc, char* argv[]) 
{
	std::cout << std::is_pod<Board>::value << std::endl;
	Board b = fen::create_board("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
	//b.init_from_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
	std::cout << b;
	int z;
	std::cin >> z;
	return 0;
}