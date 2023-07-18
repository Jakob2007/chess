
#ifndef Board_file
#define Board_file

#include <iostream>
#include <definitions.hpp>

struct Board {
	typedef std::pair<int, int> Pos;
	typedef uint8_t Sq;
	typedef int8_t Piece;
	typedef int8_t Small_num;

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
	Small_num MOVE_PATTERNS[N_PIECES][8][2];

	bool IS_MOVEMENT_CONTINUES[N_PIECES];

	char piece_names[N_PIECES];

	inline static Pos i_to_pair(Sq index);

	inline static Sq pair_to_i(const Pos& pos);

	inline static std::string pair_to_sq(Pos pair);

	inline static Pos sq_to_pair(std::string str);

	inline static Sq sq_to_i(std::string sq);

	inline static std::string i_to_sq(Sq i);

	Board copy();

	void from_fen(std::string fen);

	void try_step(Pos pos, const Small_num* pattern, Sq* moves, Sq* index, Sq* captcha, Small_num is_white, bool continues);
	
	bool try_step_check_pin(Pos pos, const Small_num* pattern, Small_num is_white, bool look_for_rook, bool same_color_piece);

	void try_step_update_pin(Pos pos, const Small_num* pattern, Small_num is_white);
	
	void check_pins();

	void is_under_attack(Sq sq, Small_num is_white, Small_num* dest);

	bool is_under_attack_reverse_check(Sq sq, Small_num is_white, const Small_num* check_pat);

	Small_num legal_pawn_moves(Sq sq, Small_num push_direction, Sq* moves);

	int legal_moves_from(Sq sq, Sq* dest, Small_num is_white);

	int all_legal_moves(int* dest, Small_num is_white);

	void do_move(Sq sq1, Sq sq2);

	int evaluate();

	int get_best_move_with_hist(int depth, int* root_move_hist);

	int get_best_move(int depth, int alpha, int beta, int* move);

	void show();

	void vsboard_loop();
};


#endif