
import chess
import pygame as pg

screen_size = [1080, 720]

WHITE = (238,237,211)
BLACK = (119,149,89)
OUTLINE_COL = (0,0,0)
BG_COL = (22,95,140)
INF_COL = (119,119,105)
ATTACK_COL = (225, 110, 80)
MARKED = (94,118,66)
PRE_MARKED = (186,200,79)

OUTLINE = 35
FIGURE_SIZE = 333
INFO_WIDTH = screen_size[0] / screen_size[1] - 1
tile_size = (screen_size[1] - 2*OUTLINE)//8

off_board_check = lambda x,y: (
	x > screen_size[1] - OUTLINE or
	x < OUTLINE or
	y > screen_size[1] - OUTLINE or
	y < OUTLINE
	)

#define fonts
pg.font.init()
font = "Marker Felt"
font = "Trebuchet MS"
font1 = pg.font.SysFont(font, 30)
font2 = pg.font.SysFont(font, 45)

# get piece ims
PiecePngs = {}
allP = pg.image.load("/Users/Jakob/documents/Python/SchachRoboter/Pieces.png")
fig_names = "KQBNRPkqbnrp"
for n in range(len(fig_names)):
	c = n%6
	r = n//6
	rect = (c*FIGURE_SIZE, r*FIGURE_SIZE, FIGURE_SIZE, FIGURE_SIZE)
	p = allP.subsurface(rect)
	PiecePngs[fig_names[n]] = p

# pygame
def setup():
	global screen, clock
	pg.display.set_caption("Schach KI")
	screen = pg.display.set_mode(screen_size, display=0)
	clock = pg.time.Clock()
	pg.init()

def draw_board(game, o):
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
		x = (s%8)*tile_size+OUTLINE+1
		y = (7-s//8)*tile_size+OUTLINE+1


		rect = (x, y, tile_size, tile_size)
		p = game.board.piece_at(s)

		# draw square
		if s == game.highlighted_square:
			pg.draw.rect(screen, MARKED, rect)
		elif s == game.hovered_square:
			pg.draw.rect(screen, PRE_MARKED, rect)
		elif s in game.pinned:
			pg.draw.rect(screen, INF_COL, rect)
		elif s in game.check_preventing:
			pg.draw.rect(screen, ATTACK_COL, rect)
		elif not (s//8 + s%8 % 2 + 1) % 2:
			pg.draw.rect(screen, BLACK, rect)
		# draw figure
		if p:
			if p.piece_type == chess.KING and game.state == "check" and p.color == game.board.turn: pg.draw.rect(screen, PRE_MARKED, rect)

			pim = pg.transform.scale(PiecePngs[p.symbol()], (tile_size, tile_size))
			screen.blit(pim, (x,y))
		
		if s in game.move_dests:
			if p: col = ATTACK_COL
			elif s == game.hovered_square: col = MARKED
			else: col = PRE_MARKED
			pg.draw.circle(screen, col, (x+tile_size/2,y+tile_size/2), tile_size/8)


def draw_last_move(move):
	if not move: return
	s1,s2 = move.from_square, move.to_square
	s = (OUTLINE + (s1%8)*tile_size + tile_size//2,
		OUTLINE + (7-s1//8)*tile_size + tile_size//2)
	e = (OUTLINE + (s2%8)*tile_size + tile_size//2,
		OUTLINE + (7-s2//8)*tile_size + tile_size//2)
	w = int(tile_size*.1 //2*2 + 1)
	pg.draw.line(screen, INF_COL, s, e, width=w)
	pg.draw.circle(screen, INF_COL, s, w//2)
	pg.draw.circle(screen, INF_COL, e, w//2)

def draw_info(game, info_offset, info_size):
	y = OUTLINE + screen_size[1]*.05
	spacing = .03
	
	# turn
	t = font1.render(game.player[game.board.turn].name, False, OUTLINE_COL)
	screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	y+= t.get_height() + screen_size[1]*spacing

	if game.state == "check":
		pg.draw.circle(screen, PRE_MARKED, (info_offset + info_size/2, y + tile_size/2), tile_size*.6)
	elif game.state == "mate":
		pg.draw.circle(screen, ATTACK_COL, (info_offset + info_size/2, y + tile_size/2), tile_size*.6)
	elif game.state == "stalemate":
		pg.draw.circle(screen, INF_COL, (info_offset + info_size/2, y + tile_size/2), tile_size*.6)
	p = pg.transform.scale(PiecePngs['K' if game.board.turn else 'k'], (tile_size, tile_size))
	screen.blit(p, (info_offset + info_size/2 - tile_size/2, y))
	y+= p.get_height() + screen_size[1]*spacing

	# ai info
	txt = game.info.replace('_', ' ')
	txt = txt.replace(':', ':\n')
	txt = txt.replace(',', ',\n')

	t = font1.render(txt, False, OUTLINE_COL)
	screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	y+= t.get_height() + screen_size[1]*spacing
	
	# move count
	t = font2.render("ZÜGE", False, OUTLINE_COL)
	screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	y+= t.get_height() + screen_size[1]*spacing*.1
	t = font2.render(str(len(game.board.move_stack)), False, OUTLINE_COL)
	screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
	y+= t.get_height() + screen_size[1]*spacing
	
	# last move
	if mv := game.last_move:
		t = font2.render(f"{mv.uci()[:2]} -> {mv.uci()[2:]}", False, OUTLINE_COL)
		screen.blit(t, (info_offset + info_size/2 - t.get_width()/2, y))
		y+= t.get_height() + screen_size[1]*spacing

def render(game):
	# draw basic
	screen.fill(BLACK)

	o = OUTLINE*.80
	info_offset = screen_size[1]
	info_size = screen_size[0] - screen_size[1]
	pg.draw.rect(screen, OUTLINE_COL, (o, o, screen_size[1] - 2*o, screen_size[1] - 2*o))
	pg.draw.rect(screen, WHITE, (OUTLINE+1, OUTLINE+1, screen_size[1] - 2*OUTLINE-2, screen_size[1] - 2*OUTLINE-2))
	pg.draw.rect(screen, OUTLINE_COL, (info_offset + o, o, info_size - 2*o, screen_size[1] - 2*o))
	pg.draw.rect(screen, WHITE, (info_offset + OUTLINE, OUTLINE, info_size - 2*OUTLINE, screen_size[1] - 2*OUTLINE))

	draw_board(game, o)

	draw_last_move(game.last_move)

	draw_info(game, info_offset, info_size)
	
	# if game.AI_info["running"]:
	# 	t = font1.render(f"AI:{game.AI_info['%']}%", False, OUTLINE_COL)
	# 	screen.blit(t, (info_offset + info_size*.7, y))
	# 	t = font1.render(str(game.AI_info['move']), False, OUTLINE_COL)
	# 	screen.blit(t, (info_offset + info_size*.71, y + t.get_height()+screen_size[1]*spacing*.1))
	# y+= p.get_height() + screen_size[1]*spacing

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

def show(game):
	x,y = pg.mouse.get_pos()
	if not off_board_check(x,y):
		x = (x - OUTLINE) // tile_size
		y = 7 - (y - OUTLINE) // tile_size
		game.hovered_square = x + y*8
	else: game.hovered_square = None

	for event in pg.event.get():
		if event.type == pg.QUIT:
			game.running = False
			return False
		elif event.type == pg.MOUSEBUTTONDOWN:
			if game.hovered_square: game.clicked = True

	render(game)

	pg.display.flip()
	clock.tick(24)
	return True

def quit():
	pg.quit()