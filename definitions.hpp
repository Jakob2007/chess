#include <chrono>

#define PAWN 1
#define ROOK 2
#define NIGHT 3
#define BISHOP 4
#define QUEEN 5
#define KING 6

#define PAWN_INDEX PAWN-1
#define ROOK_INDEX ROOK-1
#define NIGHT_INDEX NIGHT-1
#define BISHOP_INDEX BISHOP-1
#define QUEEN_INDEX QUEEN-1
#define KING_INDEX KING-1

#define N_PIECES 6
#define BOARD_SIZE 64
#define ROW_SIZE 8
#define MAX_MOVES 27
#define MAX_MOVES_BOARD 219
#define INTERRUPT_VAL 99
#define LIST_END UINT8_MAX
#define WORST_CASE 9999

#define get_sqs_from_move(move) move >> 6, move & 63
#define get_move_from_sqs(sq1, sq2) (sq1 << 6) | sq2;

#define Start_time auto start_time = std::chrono::high_resolution_clock::now();
#define End_time "( " + std::to_string( std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_time).count() ) + "ns x" + std::to_string((int)((float)1000000000 / std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_time).count())) + " )"
#define BASIC_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define HEY std::cout << "hey" << std::endl;

#define COUNTER call_count++; std::cout << "call count: " << call_count << std::endl;
int call_count = 0;