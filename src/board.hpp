
#ifndef Board_h
#define Board_h

#include <iostream>

#include "definitions.hpp"
#include "openings.hpp"

inline Pos i_to_pair(Sq index) {
	return std::make_pair(index % ROW_SIZE, index / ROW_SIZE);
}

inline Sq pair_to_i(const Pos& pos) {
	if (pos.first < 0 || pos.second < 0 || pos.first >= ROW_SIZE || pos.second >= ROW_SIZE) {
		return LIST_END;
	}
	return pos.first + pos.second * ROW_SIZE;
}

inline std::string pair_to_sq(Pos pair) {
	return std::string(1, 'a' + (7 - pair.first)) + (char)('1' + pair.second);
}

inline Pos sq_to_pair(std::string str) {
	return std::make_pair(7 - (str.at(0) - 'a'), str.at(1) - '1');
}

inline Sq sq_to_i(std::string sq) {
	return pair_to_i(sq_to_pair(sq));
}

inline std::string i_to_sq(Sq i) {
	return pair_to_sq(i_to_pair(i));
}

struct Board {
	Piece board[BOARD_SIZE];
	Small_num pins[BOARD_SIZE][2];
	Sq check_preventing[ROW_SIZE-1];
	bool is_whites_turn;
	bool possible_castles[4];
	int full_move_count;
	Sq enpassant;
	Pos white_king;
	Pos black_king;

	// PAWN, ROOK, NIGHT, BISHOP, QUEEN, KING
	const Small_num MOVE_PATTERNS[N_PIECES][8][2] = {
		{{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
		{{1,0}, {-1,0}, {0,1}, {0,-1}, {0,0}, {0,0}, {0,0}, {0,0}},
		{{2,-1}, {2,1}, {1,2}, {-1,2}, {-2,1}, {-2,-1}, {-1,-2}, {1,-2}},
		{{1,1}, {-1,1}, {-1,-1}, {1,-1}, {0,0}, {0,0}, {0,0}, {0,0}},
		{{1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}},
		{{1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}}
	};

	const bool IS_MOVEMENT_CONTINUES[N_PIECES] = {false, true, false, true, true, false};

	const char piece_names[N_PIECES] = {'p','r','n','b','q','k'};

	void from_fen(std::string fen);

	Board();

	Board(std::string fen);

	Board copy();

	void try_step(Pos pos, const Small_num* pattern, Sq* moves, Sq* index, Sq* captcha, Small_num is_white, bool continues);
	
	bool try_step_check_pin(Pos pos, const Small_num* pattern, Small_num is_white, bool look_for_rook, bool same_color_piece);
	
	void check_pins();

	void is_under_attack(Sq sq, Small_num is_white, Small_num* dest);

	bool is_under_attack_reverse_check(Sq sq, Small_num is_white, const Small_num* check_pat);

	Small_num legal_pawn_moves(Sq sq, Small_num push_direction, Sq* moves);

	int legal_moves_from(Sq sq, Sq* dest, Small_num is_white);

	int all_legal_moves(int* dest, Small_num is_white);

	void do_move(Sq sq1, Sq sq2);

	void do_move(Sq sq1, Sq sq2, Openings* op);

	int evaluate_move(Move mv);

	void sort_moves(int* moves, int n_moves);

	bool is_important_move(Move mv);

	int evaluate();

	int get_best_move_with_hist(int depth, int* root_move_hist);

	int minimax(int depth, int alpha, int beta, int deepening);

	int get_best_move(int depth, Openings* op);

	void show();

	void vsboard_loop(Openings* op);
};


#endif