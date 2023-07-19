#include <iostream>

#include "definitions.hpp"
#include "board.hpp"

void show_timed_move(Board board, Sq sq) {
	Start_time
	
	Sq moves[MAX_MOVES];
	board.legal_moves_from(sq, moves, board.board[sq] > 0 ? 1 : -1);
	
	std::cout << "move check" << End_time;
	
	std::cout << ' ' << (int)sq << '[' << board.piece_names[board.board[sq] * (board.board[sq] > 0 ? 1 : -1) - 1]  << "] -> ";

	for (Sq i=0; i<MAX_MOVES; i++) {
		if (moves[i] == LIST_END) {
			std::cout << "-";
			break;
		}
		std::cout << board.pair_to_sq(board.i_to_pair(moves[i])) << ' ' << (int)moves[i] << ' ';
	}
	std::cout << std::endl;
}

void test_fen(Board b, std::string fen, int* exp_moves) {
	b.from_fen(fen);

	int moves[MAX_MOVES_BOARD] = {0};
	Start_time
	for (int i=1; i<1; i++) b.all_legal_moves(moves, 1); // 100000
	int count = b.all_legal_moves(moves, 1);
	std::cout << End_time << std::endl;

	// for (int i=0; i<count+1; i++) {
	// 	int first = moves[i] >> 6;
	// 	int second = moves[i] & 63;
	// 	std::cout << Board::i_to_sq(first) << Board::i_to_sq(second) << ", ";
	// }

	for (int i=0; i<MAX_MOVES_BOARD; i++) {
		if (moves[i] != exp_moves[i]) {
			std::cout << "test failed: " << count << ' ';
			int first = moves[i] >> 6;
			int second = moves[i] & 63;
			std::cout << "got: " << b.i_to_sq(first) << b.i_to_sq(second);
			first = exp_moves[i] >> 6;
			second = exp_moves[i] & 63;
			std::cout << " exp: " << b.i_to_sq(first) << b.i_to_sq(second) << std::endl;
			// for (int i=0; i<count+1; i++) {
			// 	std::cout << moves[i] << ", ";
			// }
			for (int i=0; i<count+1; i++) {
				int first = moves[i] >> 6;
				int second = moves[i] & 63;
				std::cout << b.i_to_sq(first) << b.i_to_sq(second) << ", ";
			}
			break;
		}
		if (moves[i] == -1) {
			std::cout << "success: " << fen;
		}
	}
	std::cout << std::endl;
}

int main() {
	Board main_board;

	main_board.from_fen("8/8/8/k2r4/8/1b3Q2/8/5K2 w - - 0 1");
	main_board.from_fen(BASIC_FEN);

	int move = 0;
	int eval;

	for (int i=0; i<10; i++) {
		Start_time
		std::cout << i << ' ';
		eval = main_board.get_best_move(i, -WORST_CASE, WORST_CASE, &move);
		std::cout << main_board.i_to_sq(move>>6) << main_board.i_to_sq(move&63) << ' ';
		//for (int j=0; j<10; j++) std::cout << Board::i_to_sq(moves[j]>>6) << Board::i_to_sq(moves[j]&63) << '-';
		std::cout << eval << End_time << std::endl;
	}

	/* raw
	0 f3d5 995( 80942ns x11011 )
	1 f3b3 993( 368373ns x2709 )
	2 f3c3 994( 7727652ns x129 )
	3 f3b3 993( 146830790ns x6 )
	4 f3c3 995( 3250877710ns x0 )
	*/
	/* alpha beta
	0 f3d5 995( 64983ns x13626 )
	1 f3b3 993( 178601ns x5578 )
	2 f3c3 994( 2180954ns x458 )
	3 f3b3 993( 24846822ns x40 )
	4 f3c3 995( 147938581ns x6 )
	*/

	// main_board.vsboard_loop();
	
	std::cout << std::endl;

	return 0;
}