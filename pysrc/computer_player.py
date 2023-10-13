
import time
import chess

class Computer_player:
	name = "Computer"

	def __init__(self, game, _):
		self.game = game

	def await_move(self):
		sq1 = None
		while True:
			while not self.game.clicked: time.sleep(.05)
			sq = self.game.hovered_square
			self.game.clicked = False

			if not sq1:
				p = self.game.board.piece_at(sq)
				if p and p.color == self.game.board.turn:
					self.game.highlighted_square = sq
					self.game.get_possible_moves(sq)
					sq1 = sq
			elif sq in self.game.move_dests:
				self.game.highlighted_square = sq
				sq2 = sq
				break
			else:
				self.game.highlighted_square = None
				self.game.move_dests = []
				sq1 = None
				self.game.clicked = True

		self.game.highlighted_square = None
		self.game.move_dests = []

		time.sleep(.05)
		self.game.highlighted_square = None
		
		mv = chess.Move.from_uci(chess.SQUARE_NAMES[sq1] + chess.SQUARE_NAMES[sq2])

		return mv

	def push_move(self, uci):
		pass

	def quit(self):
		pass