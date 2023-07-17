
import atexit
import pygame as pg
from subprocess import Popen, PIPE, STDOUT

screen_size = [1080, 720]
pg.display.set_caption("Schach KI")
screen = pg.display.set_mode(screen_size, display=0)
clock = pg.time.Clock()
pg.init()

# define colorpelet
WHITE=(90,200,240)
BLACK=(20,105,139)
OUTLINE_COL = (0,0,0)
BG_COL = (22,95,140)
MARKED = (75,90,140)
PRE_MARKED = (68,75,100)

WHITE = (238,237,211)
BLACK = (119,149,89)
OUTLINE_COL = (0,0,0)
BG_COL = (22,95,140)
INF_COL = (119,119,105)
MARKED = (94,118,66)
PRE_MARKED = (186,200,79)

OUTLINE = 35
FIGURE_SIZE = 333
INFO_WIDTH = screen_size[0] / screen_size[1] - 1
tile_size = (screen_size[1] - 2*OUTLINE)//8

first_square, targets = None, []
hovering = None
last_move = None
pinned = []
check = False

#define fonts
pg.font.init()
font = "Marker Felt"
font = "Trebuchet MS"
font1 = pg.font.SysFont(font, 30)
font2 = pg.font.SysFont(font, 45)

# GET PIECE IMGs
PiecePngs = {}
allP = pg.image.load("/Users/Jakob/documents/Python/SchachRoboter/Pieces.png")
figNames = "KQBNRPkqbnrp"
figNames = [6, 5, 4, 3, 2, 1, -6, -5, -4, -3, -2, -1]
for n in range(len(figNames)):
	c = n%6
	r = n//6
	rect = (c*FIGURE_SIZE, r*FIGURE_SIZE, FIGURE_SIZE, FIGURE_SIZE)
	p = allP.subsurface(rect)
	PiecePngs[figNames[n]] = p

def render_move_line(move, col):
	s = (OUTLINE + (7-move[0]%8)*tile_size + tile_size//2,
		OUTLINE + (7-move[0]//8)*tile_size + tile_size//2)
	e = (OUTLINE + (7-move[1]%8)*tile_size + tile_size//2,
		OUTLINE + (7-move[1]//8)*tile_size + tile_size//2)
	w = int(tile_size*.1 //2*2 + 1)
	pg.draw.line(screen, col, s, e, width=w)
	pg.draw.circle(screen, col, s, w//2)
	pg.draw.circle(screen, col, e, w//2)


def render(vsboard):
	# draw basic
	screen.fill(BLACK)
	o = OUTLINE*.80
	info_offset = screen_size[1]
	info_size = screen_size[0] - screen_size[1]
	pg.draw.rect(screen, OUTLINE_COL, (o, o, screen_size[1] - 2*o, screen_size[1] - 2*o))
	pg.draw.rect(screen, WHITE, (OUTLINE+1, OUTLINE+1, screen_size[1] - 2*OUTLINE-2, screen_size[1] - 2*OUTLINE-2))
	pg.draw.rect(screen, OUTLINE_COL, (info_offset + o, o, info_size - 2*o, screen_size[1] - 2*o))
	pg.draw.rect(screen, WHITE, (info_offset + OUTLINE, OUTLINE, info_size - 2*OUTLINE, screen_size[1] - 2*OUTLINE))

	### draw board
	# draw text
	n = ["8","7","6","5","4","3","2","1"]
	l = ["a","b","c","d","e","f","g","h"]
	for t in range(8):
		s1 = font1.render(n[t], False, (0,0,0))
		s2 = font1.render(n[t], False, (0,0,0))
		s3 = font1.render(l[t], False, (0,0,0))
		s4 = font1.render(l[t], False, (0,0,0))
		screen.blit(s1, (o*.5 - s1.get_width()*.5, OUTLINE + t*tile_size + tile_size*.5 - s1.get_height()*.5))
		screen.blit(s2, (OUTLINE*1.25+o*.5 + tile_size*8 - s2.get_width()*.5, OUTLINE + t*tile_size + tile_size*.5 - s1.get_height()*.5))
		screen.blit(s3, (OUTLINE + t*tile_size + tile_size*.5 - s3.get_width()*.5, o*.5 - s1.get_height()*.5))
		screen.blit(s4, (OUTLINE + t*tile_size + tile_size*.5 - s4.get_width()*.5, OUTLINE*1.25+o*.5 + tile_size*8 - s1.get_height()*.5))

	# draw board
	for s in range(64):
		x = (7-s%8)*tile_size+OUTLINE+1
		y = (7-s//8)*tile_size+OUTLINE+1
		rect = (x, y, tile_size, tile_size)
		p = vsboard[s]
		# draw square
		if s == first_square:
			pg.draw.rect(screen, MARKED, rect)
		elif s == hovering:
			pg.draw.rect(screen, PRE_MARKED, rect)
		elif s in pinned or (p and s in targets):
			pg.draw.rect(screen, INF_COL, rect)
		elif not (s//8 + s%8 % 2 + 1) % 2:
			pg.draw.rect(screen, BLACK, rect)
		# draw figure
		if p * vsboard[-1] == 6 and check:
			pg.draw.rect(screen, PRE_MARKED, rect)
		if p:
			pim = pg.transform.scale(PiecePngs[p], (tile_size, tile_size))
			screen.blit(pim, (x,y))
		if s in targets and s == hovering:
			pg.draw.circle(screen, INF_COL, (x+tile_size/2, y+tile_size/2), tile_size*.125)
		elif s in targets:
			if not p:
				pg.draw.circle(screen, MARKED, (x+tile_size/2, y+tile_size/2), tile_size*.125)

	# draw last move
	if last_move: render_move_line(last_move, INF_COL)

	### draw info
	y = OUTLINE + screen_size[1]*.05
	spacing = .03
	# turn
	if check:
		pg.draw.circle(screen, PRE_MARKED, (info_offset + info_size/2, y + tile_size/2), tile_size*.6)
	p = pg.transform.scale(PiecePngs[6 * vsboard[-1]], (tile_size, tile_size))
	screen.blit(p, (info_offset + info_size/2 - tile_size/2, y))
	y+= p.get_height() + screen_size[1]*spacing
	
	# if game.AI_info["running"]:
	# 	t = font1.render(f"AI:{game.AI_info['%']}%", False, OUTLINE_COL)
	# 	screen.blit(t, (info_offset + info_size*.7, y))
	# 	t = font1.render(str(game.AI_info['move']), False, OUTLINE_COL)
	# 	screen.blit(t, (info_offset + info_size*.71, y + t.get_height()+screen_size[1]*spacing*.1))
	# y+= p.get_height() + screen_size[1]*spacing

	# # evaluation
	# txt = str(game.cur_evaluation)
	# if game.STATE in ["w", "b"]:
	# 	txt = "- Schachmatt! -"
	# elif game.STATE in ["d"]:
	# 	txt = "- Patt! -"
	# t = font2.render(txt, False, OUTLINE_COL)
	# screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	# y+= t.get_height() + screen_size[1]*spacing

	# # move count
	t = font2.render("ZÜGE", False, OUTLINE_COL)
	screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	y+= t.get_height() + screen_size[1]*spacing*.1
	t = font2.render(str(vsboard[-2]), False, OUTLINE_COL)
	screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	y+= t.get_height() + screen_size[1]*spacing
	
	# # last move
	if last_move:
		t = font2.render(f"{i_to_sq(last_move[0])} -> {i_to_sq(last_move[1])}", False, OUTLINE_COL)
		screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
		y+= t.get_height() + screen_size[1]*spacing

	# # figures
	# l = sorted(game.CAPTURED["w"])[::-1]
	# x = info_offset + info_size*.2
	# for _,fig in l:
	# 	p = pg.transform.scale(PiecePngs[fig], (tile_size*.70, tile_size*.70))
	# 	screen.blit(p, (x - p.get_width()/2, y))
	# 	x+= p.get_width()*.40
	# if l:
	# 	y+= p.get_height() + screen_size[1]*spacing*.1
	# l = sorted(game.CAPTURED["b"])[::-1]
	# x = info_offset + info_size*.2
	# for _,fig in l:
	# 	p = pg.transform.scale(PiecePngs[fig], (tile_size*.70, tile_size*.70))
	# 	screen.blit(p, (x - p.get_width()/2, y))
	# 	x+= p.get_width()*.40

def sq_to_i(sq):
	return  7 - (ord(sq[0]) - ord('a')) + (ord(sq[1]) - ord('1')) * 8

def i_to_sq(index):
	return chr(ord('a') + (7 - index % 8)) + chr(ord('1') + index // 8)

def read():
	return pipe.stdout.readline().rstrip().decode()

def end():
	pipe.communicate("brk".encode())

def send(cmd, value=""):
	pipe.stdin.write((cmd + value + "\n").encode())
	pipe.stdin.flush()
	ans = read()
	if ans != "ok":
		if ans.endswith("ok"):
			print(f"inf: {ans[:-2]}")
		else:
			raise Exception(f"error receving anser for {cmd}-{value}: {ans}")
	return ans[:-2]
	
def get_visaul_move(x, y, vsboard):
	global first_square, targets, last_move, pinned, check
	if (x > screen_size[1] - OUTLINE or x < OUTLINE or
		y > screen_size[1] - OUTLINE or y < OUTLINE):
		return
	x = 7 - (x - OUTLINE) // tile_size
	y = 7 - (y - OUTLINE) // tile_size
	s = x + y*8
	if vsboard[s] and s != first_square and vsboard[s] * vsboard[-1] > 0:
		first_square = s
		send("inf", i_to_sq(s)).split()
		mvs = read().split()
		targets = list(map(lambda mv: sq_to_i(mv), mvs))
		return
	if s not in targets:
		first_square = None
		targets = []
		return
	last_move = (first_square, s)
	inf = send("act", f"{i_to_sq(first_square)}{i_to_sq(s)}").split()
	if inf:
		if inf[0] == "check":
			check = True
			inf = inf[1:]
		else:
			check = False
		pinned = list(map(lambda mv: sq_to_i(mv), inf))
	else:
		check = False
		pinned = []
	first_square = None
	targets = []

	# inf = send("kim")
	# last_move = (sq_to_i(inf[0:2]), sq_to_i(inf[2:4]))
	# print("move evaluation: " + inf[4:])

def main():
	global pipe, hovering
	pipe = Popen(['/Users/Jakob/Documents/C++/chess/chess'], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
	ans = read()
	print(ans)
	if ans != "rdy": raise Exception("engine is not ready")
	atexit.register(end)

	# fen = "r3k2r/ppp1pppp/8/6r1/8/2B5/P1P1P2P/R3K2R"
	# if fen:
	# 	if " " in fen:
	# 		send("fen", fen)
	# 	else:
	# 		send("fen", fen+"_w_KQkq_-_0_1")

	while True:
		send("get")
		b = list(map(int, read().split()))
		render(b)
		for event in pg.event.get():
			if event.type == pg.QUIT:
				return
			elif event.type == pg.MOUSEBUTTONDOWN:
				x,y = pg.mouse.get_pos()
				get_visaul_move(x, y, b)

		x,y = pg.mouse.get_pos()
		if (x > screen_size[1] - OUTLINE or x < OUTLINE or
			y > screen_size[1] - OUTLINE or y < OUTLINE):
			hovering = None
		else:
			x = 7 - (x - OUTLINE) // tile_size
			y = 7 - (y - OUTLINE) // tile_size
			hovering = x + y*8
			
		pg.display.flip()
		clock.tick(24)
	

# g++ -o chess -w -O3 ./main.cpp; ./chess
# g++ -o chess -w -O3 ./main.cpp; python3.10 /Users/Jakob/Documents/C++/chess/show_chess.py

if __name__ == "__main__":
	main()
