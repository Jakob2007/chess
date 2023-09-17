
#include <iostream>
#include <thread>

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
		std::cout << pair_to_sq(i_to_pair(moves[i])) << ' ' << (int)moves[i] << ' ';
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
			std::cout << "got: " << i_to_sq(first) << i_to_sq(second);
			first = exp_moves[i] >> 6;
			second = exp_moves[i] & 63;
			std::cout << " exp: " << i_to_sq(first) << i_to_sq(second) << std::endl;
			// for (int i=0; i<count+1; i++) {
			// 	std::cout << moves[i] << ", ";
			// }
			for (int i=0; i<count+1; i++) {
				int first = moves[i] >> 6;
				int second = moves[i] & 63;
				std::cout << i_to_sq(first) << i_to_sq(second) << ", ";
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
	Openings main_opening;

	// while (main_opening.current_move_node != nullptr)
	// {
	// 	Sq move[2] = {0};
	// 	std::string name;
	// 	Start_time
	// 	main_opening.next_move(move, &name);
	// 	main_opening.push_move(move[0], move[1]);

	// 	std::cout << "Opening: " << name  << ' ' << i_to_sq(move[0]) << i_to_sq(move[1]) << ' ' << End_time << std::endl;

	// }

	// return 0;

	main_board.from_fen(BASIC_FEN);

	// main_board.from_fen("r3k1r1/ppp2p2/b1nq1n1p/3pp1b1/4P3/1PNPBN2/P1P1BPPP/R2QK2R w - - 0 1");

	// std::cout << main_board.evaluate();

	main_opening.current_move_node = nullptr;

	// int move = 0;
	// for (int i=0; i<6; i++) {
	// 	Start_time
	// 	std::cout << i << ' ';
	// 	int move = main_board.get_best_move(i, &main_opening);
	// 	std::cout << i_to_sq(move>>6) << i_to_sq(move&63) << ' ';
	// 	std::cout << End_time << std::endl;
	// }

	main_board.vsboard_loop(&main_opening);
	
	std::cout << std::endl;

	return 0;
}