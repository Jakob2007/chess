
#include <random>

#include "square_tables.hpp"
#include "definitions.hpp"
#include "openings.hpp"
#include "board.hpp"

int Board::evaluate_move(Move mv) {
	Sq sq1 = mv >> 6;
	Sq sq2 = mv & 63;

	Piece p = board[sq1];
	Piece cp = board[sq2];

	int eval = 0;
	const int value[N_PIECES+1] = {0,1,5,3,3,9,999};

	if (p > 0) {
		eval+= value[-cp] * PIECE_VAL_AMP;
		//eval+= SQUARE_TABLES[p-1][63-sq1] - SQUARE_TABLES[p-1][63-sq2]; // to inefficient probably because of cache locality
		if (cp) eval+= SQUARE_TABLES[(-cp)-1][mirror_sq(63-sq2)];
	}
	else {
		eval+= value[cp] * PIECE_VAL_AMP;
		//eval+= SQUARE_TABLES[(-p)-1][mirror_sq(63-sq1)] - SQUARE_TABLES[(-p)-1][mirror_sq(63-sq2)]; // to inefficient
		if (cp) eval+= SQUARE_TABLES[cp-1][63-sq2];
	}
	return eval;
}

void Board::sort_moves(int* moves, int n_moves) {
	int values[MAX_MOVES_BOARD];

	values[0] = evaluate_move(moves[0]);
	for (int i = 1; i < n_moves; i++) {
        Move mv = moves[i];
		int key = evaluate_move(mv);
		values[i] = key;
        int j = i - 1;

        while (j >= 0 && values[j] < key) {
            moves[j + 1] = moves[j];
            values[j + 1] = values[j];
            j--;
        }

        moves[j + 1] = mv;
		values[j + 1] = key;
    }
}

bool Board::is_important_move(Move mv) {
	Sq sq1 =  mv >> 6;
	Sq sq2 = mv & 63;
	// if (board[sq2] && (board[sq1] != 1 || board[sq1] != -1)) return true;
	if (board[sq2] != 0) return true;
	if (check_preventing[0] != LIST_END) return true;
	return false;
}

int Board::evaluate() {
	int piece_eval = 0;
	int position_eval = 0;
	const int value[N_PIECES+1] = {0,1,5,3,3,9,999};

	for (int i=0; i<BOARD_SIZE; i++) {
		Piece p = board[i];
		if (!p) continue;
		if (p > 0) {
			piece_eval+= value[p];
			position_eval+= SQUARE_TABLES[p-1][63-i];
		}
		else {
			piece_eval-= value[-p];
			position_eval-= SQUARE_TABLES[(-p)-1][mirror_sq(63-i)];
		}
	}

	return piece_eval * PIECE_VAL_AMP + position_eval;
}

int Board::get_best_move_with_hist(int depth, Move* root_move_hist) {
	Move moves[MAX_MOVES_BOARD] = {0};

	int move_count = all_legal_moves(moves, is_whites_turn ? 1 : -1);

	if (is_whites_turn) {
		int best_val = -WORST_CASE;
		for (int i=0; i<move_count; i++) {
			Board new_board = copy();
			new_board.do_move(get_sqs_from_move(moves[i]));
			int val;
			
			Move* move_hist = (Move*)calloc(depth + 1, sizeof(Move));
			if (depth > 0) val = new_board.get_best_move_with_hist(depth-1, move_hist) - 1;
			else val = new_board.evaluate();
			
			move_hist[depth] = moves[i];

			if (val > best_val) {
				best_val = val;
				for (int i=0; i<depth+1; i++) {
					root_move_hist[i] = move_hist[i];
				}
			}
			free(move_hist);
		}
		return best_val;
	}
	else {
		int best_val = WORST_CASE;
		for (int i=0; i<move_count; i++) {
			Board new_board = copy();
			new_board.do_move(get_sqs_from_move(moves[i]));
			int val;
			
			Move* move_hist = (Move*)calloc(depth + 1, sizeof(Move));
			if (depth > 0) val = new_board.get_best_move_with_hist(depth-1, move_hist) - 1;
			else val = new_board.evaluate();
			
			move_hist[depth] = moves[i];

			if (val < best_val) {
				best_val = val;
				for (int i=0; i<depth+1; i++) {
					root_move_hist[i] = move_hist[i];
				}
			}
			free(move_hist);
		}
		return best_val;
	}
}

int Board::minimax(int depth, int alpha, int beta, int deepening) {	
	Move moves[MAX_MOVES_BOARD] = {0};
	int move_count = all_legal_moves(moves, is_whites_turn ? 1 : -1);

	if (!move_count) {
		if (check_preventing[0] == LIST_END) return 0;
		else return is_whites_turn ? WORST_CASE : -WORST_CASE;
	}

	sort_moves(moves, move_count);

	if (is_whites_turn) {
		int best_val = WORST_CASE;
		for (int i=0; i<move_count; i++) {
			Board new_board = copy();
			new_board.do_move(get_sqs_from_move(moves[i]));

			int val;
			if (depth > 0) val = new_board.minimax(depth-1, alpha, beta, deepening) - 1;
			else if (deepening > 0 && (is_important_move(moves[i]) || move_count <= 5)) val = new_board.minimax(0, alpha, beta, deepening-1) - 1;
			else if (move_count <= 3) val = new_board.minimax(0, alpha, beta, deepening) - 1;
			else val = new_board.evaluate();

			if (val > best_val) {
				best_val = val;
			}

			alpha = std::max(alpha, best_val);
			if (alpha >= beta) {
				// Prune the search tree.
				break;
			}
		}
		return best_val;
	}
	else {
		int best_val = -WORST_CASE;
		for (int i=0; i<move_count; i++) {
			Board new_board = copy();
			new_board.do_move(get_sqs_from_move(moves[i]));

			int val;
			if (depth > 0) val = new_board.minimax(depth-1, alpha, beta, deepening) + 1;
			else if (deepening > 0 && (is_important_move(moves[i]) || move_count <= 5)) val = new_board.minimax(0, alpha, beta, deepening-1) + 1;
			else if (move_count <= 3) val = new_board.minimax(0, alpha, beta, deepening) + 1;
			else val = new_board.evaluate();

			if (val < best_val) {
				best_val = val;
			}

			beta = std::min(beta, best_val);
			if (alpha >= beta) {
				// Prune the search tree.
				break;
			}
		}
		return best_val;
	}
}

Move Board::get_best_move(int depth, int deepening, Openings* op) {
	int alpha = WORST_CASE;
	int beta = -WORST_CASE;

	if (op->current_move_node != nullptr) {
		Sq move[2] = {0};
		std::string name;
		if (op->next_move(move, &name)) {
			return get_move_from_sqs(move[0], move[1]);
		}
	}

	// Seed the random number generator with a time-based seed
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(0, 1);
	
	Move moves[MAX_MOVES_BOARD] = {0};
	int move_count = all_legal_moves(moves, is_whites_turn ? 1 : -1);
	sort_moves(moves, move_count);

	int best_move_index = -1;

	if (is_whites_turn) {
		int best_val = WORST_CASE;
		for (int i=0; i<move_count; i++) {
			Board new_board = copy();
			new_board.do_move(get_sqs_from_move(moves[i]));

			int val;
			if (depth > 0) val = new_board.minimax(depth-1, alpha, beta, deepening) - 1;
			else val = new_board.evaluate();

			if (val > best_val) {
				best_val = val;
				best_move_index = i;
			}
			if (val == best_val & distribution(gen)) {
				// 50/50 if move equal to prev best
				best_move_index = i;
			}

			alpha = std::max(alpha, best_val);
			if (alpha >= beta) {
				// Prune the search tree.
				break;
			}
		}
		return moves[best_move_index];
	}
	else {
		int best_val = -WORST_CASE;
		for (int i=0; i<move_count; i++) {
			Board new_board = copy();
			new_board.do_move(get_sqs_from_move(moves[i]));

			int val;
			if (depth > 0) val = new_board.minimax(depth-1, alpha, beta, deepening) + 1;
			else val = new_board.evaluate();

			if (val < best_val) {
				best_val = val;
				best_move_index = i;
			}
			if (val == best_val & distribution(gen)) {
				// 50/50 if move equal to prev best
				best_move_index = i;
			}

			beta = std::min(beta, best_val);
			if (alpha >= beta) {
				// Prune the search tree.
				break;
			}
		}
		return moves[best_move_index];
	}
}