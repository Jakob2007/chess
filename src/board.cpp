#include <iostream>
#include <math.h>

#include "definitions.hpp"
#include "openings.hpp"
#include "board.hpp"

void Board::from_fen(std::string fen) {
	for (Sq i=0; i<BOARD_SIZE; i++) {
		board[i] = 0;
	}

	// get board fen / dump to board
	Sq index = BOARD_SIZE-1;
	for (int i=0; i<fen.length(); i++) {
		char val = fen.at(i);
		if (val == ' ' || val == '_') {
			index = i;
			break;
		}
		if (val == '/') continue;
		if ('0' < val && val < '9') {
			index-= val - '0';
			continue;
		}
		if ('B' <= val && val <= 'R') {
			for (Piece i=0; i<N_PIECES; i++) {
				if (std::tolower(val) == piece_names[i]) {
					board[index] = i+1;
					index--;
					break;
				}
			}
		} else {
			for (Piece i=0; i<N_PIECES; i++) {
				if (val == piece_names[i]) {
					board[index] = -(i+1);
					index--;
					break;
				}
			}
		}
	}

	is_whites_turn = (fen.at(index + 1) == 'w');

	index+= 3;

	// possible castles
	for (Small_num i=0; i<4; i++) possible_castles[i] = false;
	if (fen.at(index) != '-') {
		for (int i=index; i<index+4; i++) {
			index++;
			char chr = fen.at(i);
			if (chr == ' ' || chr == '_') break;
			if (chr == 'K') possible_castles[0] = true;
			if (chr == 'Q') possible_castles[1] = true;
			if (chr == 'k') possible_castles[2] = true;
			if (chr == 'q') possible_castles[3] = true;
		}
	} else index+=2;

	enpassant = LIST_END;
	if (fen.at(index+1) != '-') {
		enpassant = sq_to_i(fen.substr(index, 2));
		index++;
	}

	full_move_count = 0;
	for (Small_num i=0; i<5; i++) {
		int num = fen.at(fen.length()-i-1);
		if (num == ' ' || num == '_') break;
		full_move_count+= pow(10, i) * (num-'0');
	}

	// check for king pins
	for (Sq i=0; i<BOARD_SIZE; i++) {
		if (board[i] == KING || board[i] == -KING) {
			if (board[i] == KING) white_king = i_to_pair(i);
			else black_king = i_to_pair(i);
		}
	}
	check_pins();

	Pos pos = is_whites_turn ? white_king : black_king;
	Small_num is_white = is_whites_turn ? 1 : -1;
	Small_num check_pat[2] = {0};
	is_under_attack(pair_to_i(pos), is_white, check_pat);
	if (check_pat[0] != INTERRUPT_VAL) {
		// std::cout << "check";
		if (is_under_attack_reverse_check(pair_to_i(pos), is_white, check_pat)) check_preventing[0] = INTERRUPT_VAL; // check for double check (cant intercept with other piece)
		else {
			for (Small_num i=0; i<7; i++) {
				pos.first+= check_pat[0];
				pos.second+= check_pat[1];
				check_preventing[i] = pair_to_i(pos);
				if (board[check_preventing[i]]) break;
				if (check_preventing[i] == LIST_END) break;
			}
		}
	} 
	else check_preventing[0] = LIST_END;
}

Board::Board() {
	// do nothing
}

Board::Board(std::string fen) {
	from_fen(fen);
}

Board Board::copy() {
	Board b;
	for (Sq i=0; i<BOARD_SIZE; i++) {
		b.board[i] = board[i];
		b.pins[i][0] = pins[i][0];
		b.pins[i][1] = pins[i][1];
	}
	for (Small_num i=0; i<ROW_SIZE-1; i++) b.check_preventing[i] = check_preventing[i];
	for (Small_num i=0; i<4; i++) b.possible_castles[i] = possible_castles[i];

	b.is_whites_turn = is_whites_turn;
	b.full_move_count = full_move_count;
	b.enpassant = enpassant;
	b.white_king = white_king;
	b.black_king = black_king;
	return b;
}

void Board::try_step(Pos pos, const Small_num* pattern, Sq* moves, Sq* index, Sq* captcha, Small_num is_white, bool continues) {
	Small_num xpat = pattern[0];
	Small_num ypat = pattern[1];

	while (true) {
		pos.first+= xpat;
		pos.second+= ypat;

		Sq new_index = pair_to_i(pos);

		if (new_index == LIST_END) return; // not on board

		Piece piece = board[new_index];

		if (!piece && continues) { // empty square
			moves[*index] = new_index;
			(*index)++;
			continue;
		} 
		if (piece * is_white > 0) return; // same color
		if (piece * is_white < 0) *captcha = new_index; // different color
		
		moves[*index] = new_index;
		(*index)++;
		return;
	}
}

bool Board::try_step_check_pin(Pos pos, const Small_num* pattern, Small_num is_white, bool look_for_rook, bool same_color_piece) {
	Pos new_pos = std::make_pair(pos.first + pattern[0], pos.second + pattern[1]);
	Sq new_index = pair_to_i(new_pos);
	
	if (new_index == LIST_END) return false; // not on board

	Piece piece = board[new_index];
	if (piece * is_white < 0) { // different color
		return -(piece * is_white) == QUEEN || (-(piece * is_white) == ROOK && look_for_rook) || (-(piece * is_white) == BISHOP && !look_for_rook);
	}
	if (piece * is_white > 0) { // same color
		if (!same_color_piece) return false;
		if (try_step_check_pin(new_pos, pattern, is_white, look_for_rook, false)) {
			// pined
			pins[new_index][0] = pattern[0];
			pins[new_index][1] = pattern[1];
		} 
		return false;
	}
	return try_step_check_pin(new_pos, pattern, is_white, look_for_rook, same_color_piece);
}

void Board::check_pins() {
	for (Sq i=0; i<BOARD_SIZE; i++) {
		pins[i][0] = 0;
		pins[i][1] = 0;
	}

	for (Small_num i=0; i<4; i++) {
		try_step_check_pin(white_king, MOVE_PATTERNS[ROOK_INDEX][i], 1, true, true);
		try_step_check_pin(white_king, MOVE_PATTERNS[BISHOP_INDEX][i], 1, false, true);
		try_step_check_pin(black_king, MOVE_PATTERNS[ROOK_INDEX][i], -1, true, true);
		try_step_check_pin(black_king, MOVE_PATTERNS[BISHOP_INDEX][i], -1, false, true);
	}
}

void Board::is_under_attack(Sq sq, Small_num is_white, Small_num* dest) {
	// Pawn attack
	for (Small_num i=-1; i<2; i+=2) {
		if (-(board[sq + is_white*8+i] * is_white) == PAWN) {
			dest[0] = i;
			dest[1] = is_white;
			return;
		}
	}

	Sq moves[MAX_MOVES];

	// captchas with try step
	for (Piece piece=1; piece<N_PIECES; piece++) {
		if (piece == QUEEN_INDEX) continue;
		Sq index = 0;
		for (Small_num i=0; i<4; i++) {
			Sq captcha = LIST_END;
			try_step(i_to_pair(sq), MOVE_PATTERNS[piece][i], moves, &index, &captcha, is_white, IS_MOVEMENT_CONTINUES[piece]);
			
			// check if any piece is thread
			if (captcha >= 0) {
				if (-(board[captcha] * is_white)-1 == piece) {
					dest[0] = MOVE_PATTERNS[piece][i][0];
					dest[1] = MOVE_PATTERNS[piece][i][1];
					return;
				}
				else if ((piece == ROOK_INDEX || piece == BISHOP_INDEX) && -(board[captcha] * is_white) == QUEEN) { // check for queen
					dest[0] = MOVE_PATTERNS[piece][i][0];
					dest[1] = MOVE_PATTERNS[piece][i][1];
					return;
				} 
			}
		}
		// check if piece has 8 moves
		if (MOVE_PATTERNS[piece][4][0] != 0 || MOVE_PATTERNS[piece][4][1] != 0) {
			for (Small_num i=4; i<8; i++) {
				Sq captcha = LIST_END;
				try_step(i_to_pair(sq), MOVE_PATTERNS[piece][i], moves, &index, &captcha, is_white, IS_MOVEMENT_CONTINUES[piece]);
				
				// check if any piece is thread
				if (captcha >= 0) {
					if (-(board[captcha] * is_white)-1 == piece) {
						dest[0] = MOVE_PATTERNS[piece][i][0];
						dest[1] = MOVE_PATTERNS[piece][i][1];
						return;
					}
					else if ((piece == ROOK_INDEX || piece == BISHOP_INDEX) && -(board[captcha] * is_white) == QUEEN) { // check for queen
						dest[0] = MOVE_PATTERNS[piece][i][0];
						dest[1] = MOVE_PATTERNS[piece][i][1];
						return;
					} 
				}
			}
		}

	}
	dest[0] = INTERRUPT_VAL;
}

bool Board::is_under_attack_reverse_check(Sq sq, Small_num is_white, const Small_num* check_pat) {
	Sq moves[MAX_MOVES];

	// captchas with try step
	for (Piece piece=BISHOP_INDEX; piece>0; piece--) { // doesnt check queen and king
		Sq index = 0;
		// check if piece has 8 moves
		if (MOVE_PATTERNS[piece][4][0] != 0 || MOVE_PATTERNS[piece][4][1] != 0) {
			for (Small_num i=7; i>3; i--) {
				if (MOVE_PATTERNS[piece][i][0] == check_pat[0] && MOVE_PATTERNS[piece][i][1] == check_pat[1]) return false;
				
				Sq captcha = LIST_END;
				try_step(i_to_pair(sq), MOVE_PATTERNS[piece][i], moves, &index, &captcha, is_white, IS_MOVEMENT_CONTINUES[piece]);
				
				// check if any piece is thread
				if (captcha >= 0) {
					if (-(board[captcha] * is_white)-1 == piece) {
						return true;
					}
					else if ((piece+1 == ROOK || piece+1 == BISHOP) && -(board[captcha] * is_white) == QUEEN) { // check for queen
						return true;
					} 
				}
			}
		}
		for (Small_num i=3; i>=0; i--) {
			if (MOVE_PATTERNS[piece][i][0] == check_pat[0] && MOVE_PATTERNS[piece][i][1] == check_pat[1]) return false;
			
			Sq captcha = LIST_END;
			try_step(i_to_pair(sq), MOVE_PATTERNS[piece][i], moves, &index, &captcha, is_white, IS_MOVEMENT_CONTINUES[piece]);
			
			// check if any piece is thread
			if (captcha >= 0) {
				if (-(board[captcha] * is_white)-1 == piece) {
					return true;
				}
				else if ((piece+1 == ROOK || piece+1 == BISHOP) && -(board[captcha] * is_white) == QUEEN) { // check for queen
					return true;
				} 
			}
		}

	}

	// cant do double check with Pawn
	return false;
}

Small_num Board::legal_pawn_moves(Sq sq, Small_num push_direction, Sq* moves) {
	Sq index = 0;

	Small_num xpin = pins[sq][0];
	Small_num ypin = pins[sq][1];

	if (!board[sq + push_direction*8] && !xpin) { // push
		moves[0] = sq + push_direction*8;
		index = 1;
		if (!board[sq + push_direction*8*2] && sq / ROW_SIZE == (push_direction > 0 ? 1 : 6)) { // second push
			moves[1] = sq + push_direction*8*2;
			index = 2;
		}
	}
	for (Small_num i=-1; i<2; i+=2) { // captcha
		Sq nsq = sq + push_direction*8+i;
		if (nsq/8 != sq/8 + push_direction) continue;
		if ((xpin == 0 && ypin == 0) || (xpin == i && ypin == push_direction)) {
			if(board[nsq] * push_direction < 0 || (nsq) == enpassant) {
				moves[index] = nsq;
				index++;
			}
		}
	}

	if (check_preventing[0] != LIST_END) {
		for (Sq i=0; i<index; i++) {
			Sq mv = INTERRUPT_VAL;
			for (Small_num j=0; j<7; j++) {
				if (check_preventing[j] == LIST_END) break;
				if (moves[i] == check_preventing[j]) {
					mv = moves[i];
					break;
				}
			}
			moves[i] = mv;
		}
	}

	moves[index] = LIST_END;
	return index;
}

int Board::legal_moves_from(Sq sq, Sq* dest, Small_num is_white) {
	Piece piece = board[sq]*is_white;
	if (piece <= 0 || (piece != KING && check_preventing[0] == INTERRUPT_VAL)) { // no piece
		dest[0] = LIST_END;
		return 0;
	}

	// pawn moves
	if (piece == PAWN) {
		return legal_pawn_moves(sq, is_white, dest);
	}

	Sq index = 0;
	Sq captcha;
	//    basic moves
	// check first bit of move pattern

	const Small_num (*fig_pat)[2] = MOVE_PATTERNS[piece-1];
	Small_num xpin = pins[sq][0];
	Small_num ypin = pins[sq][1];
	bool no_pin = (xpin == 0 && ypin == 0);
	bool is_contiunues = IS_MOVEMENT_CONTINUES[piece-1];
	for (Small_num i=0; i<4; i++) {
		const Small_num* pat = fig_pat[i];
		Small_num pat_first = pat[0];
		Small_num pat_second = pat[1];
		if (no_pin ||
		(xpin == pat_first && ypin == pat_second) ||
		(xpin == -pat_first && ypin == -pat_second)) {
			try_step(i_to_pair(sq), pat, dest, &index, &captcha, is_white, is_contiunues);
		}
	}
	// check rest if nesecery
	if (MOVE_PATTERNS[piece-1][4][0] != 0 || MOVE_PATTERNS[piece-1][4][1] != 0) {
		for (Small_num i=4; i<8; i++) {
			const Small_num* pat = fig_pat[i];
			Small_num pat_first = pat[0];
			Small_num pat_second = pat[1];
			if (no_pin ||
			(xpin == pat_first && ypin == pat_second) ||
			(xpin == -pat_first && ypin == -pat_second)) {
				try_step(i_to_pair(sq), pat, dest, &index, &captcha, is_white, is_contiunues);
			}
		}
	}

	// check for castle
	if (check_preventing[0] == LIST_END && piece == KING) {
		Small_num pat[2] = {};
		if (is_whites_turn) {
			is_under_attack(2, 1, pat);
			if (pat[0] == INTERRUPT_VAL && !board[1] && !board[2] && possible_castles[0]) {
				dest[index] = 1;
				index++;
			}
			is_under_attack(4, 1, pat);
			if (pat[0] == INTERRUPT_VAL && !board[4] && !board[5] && !board[6] && possible_castles[1]) {
				dest[index] = 5;
				index++;
			}
		}
		else {
			is_under_attack(58, -1, pat);
			if (pat[0] == INTERRUPT_VAL && !board[57] && !board[58] && possible_castles[2]) {
				dest[index] = 57;
				index++;
			}
			is_under_attack(60, -1, pat);
			if (pat[0] == INTERRUPT_VAL && !board[60] && !board[61] && !board[62] && possible_castles[3]) {
				dest[index] = 61;
				index++;
			}
		}
	}
	// prevent king from emo
	if (piece == KING) {
		board[sq] = 0;
		for (Sq i=0; i<index; i++) {
			Small_num check[2] = {0};
			is_under_attack(dest[i], is_white, check);
			if (check[0] != INTERRUPT_VAL) dest[i] = INTERRUPT_VAL; // is_under attack
		}
		board[sq] = piece*is_white;
	}
	// check if piece is protecting king from current check
	else if (check_preventing[0] != LIST_END) {
		for (Sq i=0; i<index; i++) {
			Sq mv = INTERRUPT_VAL;
			for (Small_num j=0; j<7; j++) {
				if (check_preventing[j] == LIST_END) break;
				if (dest[i] == check_preventing[j]) {
					mv = dest[i];
					break;
				}
			}
			dest[i] = mv;
		}
	}

	dest[index] = LIST_END;
	return index;
}

int Board::all_legal_moves(Move* dest, Small_num is_white) {
	int index = 0;
	Sq moves[MAX_MOVES] = {};
	for (Sq i=0; i<BOARD_SIZE; i++) {
		int new_index = legal_moves_from(i, moves, is_white);
		for (int j=0; j<new_index; j++) {
			if (moves[j] == INTERRUPT_VAL) continue; // move is not check preventing
			// std::cout << i_to_sq(i) << i_to_sq(moves[j]) << ' ';
			dest[index] = get_move_from_sqs(i, moves[j]);
			index++;
		}
	}
	dest[index] = -1;
	return index;
}

void Board::do_move(Sq sq1, Sq sq2) {
	Small_num is_white = is_whites_turn ? 1 : -1;
	is_whites_turn = !is_whites_turn;
	if (is_whites_turn) full_move_count++;
	Piece piece = board[sq1];

	// do move
	board[sq1] = 0;
	board[sq2] = piece;

	enpassant = LIST_END;
	if (piece * is_white == PAWN) {
		// update enpassant
		if (sq2 == sq1 + is_white*8*2) enpassant = sq1 + is_white*8;
		// check promotion
		if (sq2/8 == 7 * !is_whites_turn) board[sq2] = QUEEN * is_white;
	}

	// check check
	Pos pos = is_whites_turn ? white_king : black_king;
	Small_num check_pat[2] = {0};
	Sq index = pair_to_i(pos);
	is_under_attack(index, -is_white, check_pat);
	if (check_pat[0] != INTERRUPT_VAL) {
		if (is_under_attack_reverse_check(index, -is_white, check_pat)) check_preventing[0] = INTERRUPT_VAL; // check for double check (cant intercept with other piece)
		else {
			for (Small_num i=0; i<7; i++) {
				pos.first+= check_pat[0];
				pos.second+= check_pat[1];
				check_preventing[i] = pair_to_i(pos);
				if (board[check_preventing[i]] && i != 6) {
					check_preventing[i+1] = LIST_END;
					break;
				}
				if (check_preventing[i] == LIST_END) break;
			}
		}
	} 
	else check_preventing[0] = LIST_END;
	
	// check castle
	if (sq1 == 0 || sq2 == 0) possible_castles[0] = false; // king side w
	else if (sq1 == 7 || sq2 == 7) possible_castles[1] = false; // queen side w
	else if (sq1 == 56 || sq2 == 56) possible_castles[2] = false; // king side b
	else if (sq1 == 63 || sq2 == 63) possible_castles[3] = false; // queen side b
	else if (sq1 == 3) { // w king
		possible_castles[0] = false;
		possible_castles[1] = false;
		if (sq2 == 1) { // perf castle
			board[0] = 0;
			board[2] = ROOK;
		}
		else if (sq2 == 5) {
			board[7] = 0;
			board[4] = ROOK;
		}
	}
	else if (sq1 == 59) { // w king
		possible_castles[2] = false;
		possible_castles[3] = false;
		if (sq2 == 57) { // perf castle
			board[56] = 0;
			board[58] = -ROOK;
		}
		else if (sq2 == 61) {
			board[63] = 0;
			board[60] = -ROOK;
		}
	}
	
	// update pins
	if (piece * is_white == KING) {
		if (piece == KING) white_king = i_to_pair(sq2);
		else black_king = i_to_pair(sq2);
	}
	check_pins();
}

void Board::do_move(Sq sq1, Sq sq2, Openings* op) {

	op->push_move(sq1, sq2);

	Small_num is_white = is_whites_turn ? 1 : -1;
	is_whites_turn = !is_whites_turn;
	if (is_whites_turn) full_move_count++;
	Piece piece = board[sq1];

	// do move
	board[sq1] = 0;
	board[sq2] = piece;

	enpassant = LIST_END;
	if (piece * is_white == PAWN) {
		// update enpassant
		if (sq2 == sq1 + is_white*8*2) enpassant = sq1 + is_white*8;
		// check promotion
		if (sq2/8 == 7 * !is_whites_turn) board[sq2] = QUEEN * is_white;
	}

	// check check
	Pos pos = is_whites_turn ? white_king : black_king;
	Small_num check_pat[2] = {0};
	Sq index = pair_to_i(pos);
	is_under_attack(index, -is_white, check_pat);
	if (check_pat[0] != INTERRUPT_VAL) {
		std::cout << "check ";
		if (is_under_attack_reverse_check(index, -is_white, check_pat)) check_preventing[0] = INTERRUPT_VAL; // check for double check (cant intercept with other piece)
		else {
			for (Small_num i=0; i<7; i++) {
				pos.first+= check_pat[0];
				pos.second+= check_pat[1];
				check_preventing[i] = pair_to_i(pos);
				if (board[check_preventing[i]] && i != 6) {
					check_preventing[i+1] = LIST_END;
					break;
				}
				if (check_preventing[i] == LIST_END) break;
			}
		}
	} 
	else check_preventing[0] = LIST_END;
	for (Small_num i=0; i<7; i++) {
		if (check_preventing[i] == LIST_END) break;
		std::cout << i_to_sq(check_preventing[i]) << "c ";
	}
	
	// check castle
	if (sq1 == 0 || sq2 == 0) possible_castles[0] = false; // king side w
	else if (sq1 == 7 || sq2 == 7) possible_castles[1] = false; // queen side w
	else if (sq1 == 56 || sq2 == 56) possible_castles[2] = false; // king side b
	else if (sq1 == 63 || sq2 == 63) possible_castles[3] = false; // queen side b
	else if (sq1 == 3) { // w king
		possible_castles[0] = false;
		possible_castles[1] = false;
		if (sq2 == 1) { // perf castle
			board[0] = 0;
			board[2] = ROOK;
		}
		else if (sq2 == 5) {
			board[7] = 0;
			board[4] = ROOK;
		}
	}
	else if (sq1 == 59) { // w king
		possible_castles[2] = false;
		possible_castles[3] = false;
		if (sq2 == 57) { // perf castle
			board[56] = 0;
			board[58] = -ROOK;
		}
		else if (sq2 == 61) {
			board[63] = 0;
			board[60] = -ROOK;
		}
	}
	
	// update pins
	if (piece * is_white == KING) {
		if (piece == KING) white_king = i_to_pair(sq2);
		else black_king = i_to_pair(sq2);
	}
	check_pins();
	for (Sq i=0; i<BOARD_SIZE; i++) {
		if (pins[i][0] || pins[i][1]) std::cout << i_to_sq(i) << "p ";
	}
}

void Board::show() {
	char white = '*';
	char black = '.';

	std::string out(BOARD_SIZE, white);

	for (Sq i=0; i<BOARD_SIZE; i++) {
		if (board[i] < 0) { // black
			out[i] = piece_names[-board[i]-1];
		} else if (board[i] > 0) { // white
			out[i] = std::toupper(piece_names[board[i]-1]);
		}
	}

	for (Sq i=BOARD_SIZE; i>0; i-=ROW_SIZE) {
		out.insert(i, "\n");
	}

	for (Sq i=0; i<BOARD_SIZE+ROW_SIZE; i+=2) {
		if (out[i] == white) { 
			out[i] = black;
		}
	}

	for (int i=out.size(); i>-1; i-=1) {
		if (i%(ROW_SIZE+1)){
			out.insert(i, " ");
		}
	}
	
	std::cout << out;

	std::cout << (is_whites_turn ? 'w' : 'b');
	std::cout << ' ' << full_move_count << std::endl;
}

void Board::vsboard_loop(Openings* op) {
	std::cout << "rdy" << std::endl;

	std::string data;
	while (true) {
		std::cin >> data;

		std::string head = data.substr(0,3);
		std::string body = data.substr(3, data.length() - 3);

		if (head == "brk") break;
		if (head == "fen") {
			from_fen(body);
			std::cout << "ok" << std::endl;
		}
		else if (head == "inf") {
			Sq moves[MAX_MOVES];
			legal_moves_from(sq_to_i(body), moves, board[sq_to_i(body)] > 0 ? 1 : -1);

			for (Sq i=0; i<MAX_MOVES; i++) {
				if (moves[i] == LIST_END) break;
				else if (moves[i] == INTERRUPT_VAL) continue;
				std::cout << i_to_sq(moves[i]) << ' ';
			}
			std::cout << "ok" << std::endl;
		}
		else if (head == "act") {
			do_move(sq_to_i(body.substr(0, 2)), sq_to_i(body.substr(2, 2)), op);
			std::cout << "ok" << std::endl;
		}
		else if (head == "get") {			
			std::cout << "ok" << std::endl;
			for (Sq i=0; i<BOARD_SIZE; i++) {
				std::cout << (int)board[i] << ' ';
			}
			if (op->current_move_node) std::cout << op->current_move_node->current_name << ' ';
			else std::cout << evaluate() << ' ';
			std::cout << full_move_count << ' ' << (is_whites_turn ? 1 : -1);
			std::cout << std::endl;
		}
		else if (head == "evl") {
			if (op->current_move_node) std::cout << op->current_move_node->current_name << '.';
			else std::cout << evaluate() << '.';

			int moves[MAX_MOVES_BOARD];
			int p_moves = all_legal_moves(moves, is_whites_turn ? 1 : -1);
			
			std::string text = "ongoing";
			if (!p_moves) {
				if (check_preventing[0] == LIST_END) text = "stalemate";
				else text = "mate";
			}
			else if (check_preventing[0] != LIST_END)  {
				text = "check";
			}
			std::cout << text << '.';

			for (int i=0; i<7; i++) {
				if (check_preventing[i] == LIST_END) break;
				std::cout << i_to_sq(check_preventing[i]) << ' ';
			}
			std::cout << '.';

			for (int i=0; i<BOARD_SIZE; i++) {
				if (pins[i][0] || pins[i][1]) {
					std::cout << i_to_sq(i) << ' ';
				}
			}
			std::cout << "ok" << std::endl;
		}
		else if (head == "kim") {
			// Start_time
			int move = get_best_move(KI_DEPTH, AI_DEEPENING, op);
			// std::cout << End_time << std::endl;
			// do_move(get_sqs_from_move(move), op);
			std::cout << i_to_sq(move>>6) << i_to_sq(move&63);
			std::cout << "ok" << std::endl;
		}
		else {
			std::cout << "err" << std::endl;
		}
	}
}
