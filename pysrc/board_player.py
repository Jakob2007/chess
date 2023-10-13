
import time
import chess
from threading import Thread

class Schrobot:
	sq_size = 100
	y_compensation = 1

	def await_board(self):
		# wait for board to finish
		time.sleep(2)

		self.busy = False

	def send(self, msg):
		if self.busy:
			time.sleep(.5)
		
		self.busy = True
		print(msg)

		t = Thread(target=self.await_board)
		t.start()

	def set_magnet(self, val):
		self.send(f"m{val};")

	def home(self):
		self.set_magnet(0)
		self.send("h;")
		self.x = 0
		self.y = 0

	def __init__(self):
		self.available = False
		self.busy = False
		if self.available:
			self.home()

	def move_steps(self, steps_a, steps_b):
		true_steps_b = steps_a*Schrobot.y_compensation + steps_b

		dir_a = 1 if steps_a >= 0 else -1
		dir_b = 1 if true_steps_b >= 0 else -1

		self.send(f"s{dir_a},{steps_a*dir_a},{dir_b},{true_steps_b*dir_b};")

		self.x+= steps_a
		self.y+= steps_b

	def push(self, from_square, to_square, fig):
		if not self.available: return
		col = 1 if fig.color == 1 else -1

		sq_x, sq_y = from_square%8, from_square//8
		steps_a, steps_b = sq_x - self.x, sq_y - self.y
		self.move_steps(steps_a, steps_b)

		self.set_magnet(col)

		if fig.piece_type == chess.KNIGHT:
			# do stuff
			return
		
		sq_x, sq_y = to_square%8, to_square//8
		steps_a, steps_b = sq_x - self.x, sq_y - self.y
		self.move_steps(steps_a, steps_b)

		self.set_magnet(0)

	def request_state(self):
		self.send(f"r;")
		return [0] * 64
	
	def quit(self):
		if self.available:
			self.send(f"q;")

class Board_player:
	name = "Board"

	def __init__(self, game, _):
		self.game = game

	def await_move(self):
		state = self.game.schrobot.request_state()

		anomalies = []

		for i,fig in enumerate(self.game.board):
			if fig > 0: fig_mark = 1
			elif fig < 0: fig_mark = -1
			else: fig_mark = 0

			if fig_mark != state[i]:
				anomalies.append((i,state[i]))
		
		if len(anomalies) == 4: # castle
			return "mv"
		elif len(anomalies) == 3: # enpassant
			return "mv"
		elif len(anomalies) == 2:
			if anomalies[0][1] == 0:
				return chess.Move(anomalies[0][0], anomalies[1][0])
			return chess.Move(anomalies[1][0], anomalies[0][0])

	def push_move(self, mv):
		# move is pushed separately
		pass

	def quit(self):
		# quiting is handelt separately
		pass