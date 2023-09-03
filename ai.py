
import time
import chess
from subprocess import Popen, PIPE, STDOUT

class AI_handler:
	def read(self):
		return self.pipe.stdout.readline().rstrip().decode()
	
	def send(self, cmd, value=""):
		self.pipe.stdin.write((str(cmd) + str(value) + "\n").encode())
		self.pipe.stdin.flush()
		
		ans = self.read()
		if ans != "ok":
			if ans.endswith("ok"): pass #print(f"inf({cmd}): {ans[:-2]}")
			else: raise Exception(f"error receving anser for {cmd}-{value}: {ans}")
		return ans[:-2]

	def quit(self):
		self.pipe.communicate("brk".encode())

	def __init__(self):
		self.info = ""

		self.pipe = Popen(['/Users/Jakob/Documents/C++/chess/chess'], stdout=PIPE, stdin=PIPE, stderr=STDOUT)

		ans = self.read()
		if ans != "rdy": raise Exception("engine is not ready: " + ans)
		
	def i_to_sq(index):
		return chr(ord('a') + (7 - index % 8)) + chr(ord('1') + index // 8)

	def sq_to_i(sq):
		return (ord(sq[0]) - ord('a')) + (ord(sq[1]) - ord('1')) * 8

	def get_evaluation(self, game):
		ans = self.send("evl").split('.')
		game.info, game.state, check_preventing, pinned = ans
		game.check_preventing = list(map(AI_handler.sq_to_i, check_preventing.split(' ')[:-1]))
		game.pinned = list(map(AI_handler.sq_to_i, pinned.split(' ')[:-1]))

	def get_possible_moves(self, i):
		return list(map(AI_handler.sq_to_i, self.send("inf", chess.SQUARE_NAMES[i]).split()))

	def push(self, uci):
		self.send("act", uci)

class AI_player:
	name = "AI"

	def __init__(self, game, _):
		self.game = game

	def await_move(self):
		resp = self.game.AI.send("kim")
		time.sleep(.05)
		return chess.Move.from_uci(resp)

	def push_move(self, uci):
		# pushing to ai is handeled by AI_handler
		pass

	def quit(self):
		# quiting is handeled by AI_handler
		pass