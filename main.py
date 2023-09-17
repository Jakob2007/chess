
import chess
import atexit
from threading import Thread
from datetime import datetime

from ai import AI_handler, AI_player
from board_player import Board_player
from computer_player import Computer_player
from lichess_player import Lichess_player, Stockfish_player

import rendering

class Game:
	player_ids = {
		"bp": Board_player,
		"li": Lichess_player,
		"stockfish": Stockfish_player,
		"cp": Computer_player,
		"ai": AI_player,
	}

	def quit(self):
		self.pl1.quit()
		self.pl2.quit()

		rendering.quit()

	def __init__(self):
		self.running = True
		
		self.clicked = False
		self.hovered_square = None
		self.highlighted_square = None
		self.move_dests = []
		
		self.info = ""

		self.last_move = None
		self.state = None
		self.check_preventing = []
		self.pinned = []

		self.li_time = 10
		self.li_incr = 3

		print('\n'.join([f"{key}: {self.player_ids[key]}" for key in self.player_ids]))

		short1 = input("white player > ")
		short2 = input("black player > ")

		self.pl1 = self.player_ids[short1.lower()](self, 1)
		print(f"White plays from {self.pl1.name}")

		self.pl2 = self.player_ids[short2.lower()](self, 0)
		print(f"Black plays from {self.pl2.name}")

		self.player = [self.pl2, self.pl1]

		rendering.setup()

		atexit.register(self.quit)

		self.board = chess.Board()

		self.AI = AI_handler()
        
	def get_possible_moves(self, sq1):
		self.move_dests = self.AI.get_possible_moves(sq1)

	def do_move(self):
		pl = self.player[ self.board.turn ]
		print(f"waiting for {pl.name}", end=" ")
		mv = pl.await_move()
		if not mv:
			self.running = False
			return
		print(mv.uci())
		print('')

		self.board.push(mv)
		self.last_move = mv

		self.AI.push(mv.uci())
		self.player[ self.board.turn ].push_move(mv)

		self.AI.get_evaluation(self)
		if self.state == "mate" or self.state == "stalemate":
			self.running = False

def move_loop(game):
	while game.running:
		game.do_move()

def main():
	game = Game()

	move_thr = Thread(target=move_loop, args=(game,))
	move_thr.daemon = True
	move_thr.start()

	while game.running:
		window = rendering.show(game)

	if len(game.board.move_stack) > 8:
		path = f"/Users/Jakob/Documents/C++/chess/logs/{datetime.today().strftime('%d.%b_%H-%Muhr')}#{game.pl1.name}Vs{game.pl2.name}.log"
		with open(path, "w") as file:
			file.write(f"{game.state}\n")
			for i, mv in enumerate(game.board.move_stack):
				if (i+1)%2:
					file.write(f"{i//2+1}. {mv.uci()} \t")
				else:
					file.write(f"{mv.uci()}\n")

	while rendering.show(game) and window: pass
	print('')

if __name__ == "__main__":
    main()