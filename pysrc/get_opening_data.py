import chess
import pandas

path = "openings"

frames = []
for section in ('a', 'b', 'c', 'd', 'e'):
	frames.append(
		pandas.read_csv(f"https://raw.githubusercontent.com/lichess-org/chess-openings/master/{section}.tsv",
		sep="\t", names=['eco', 'name', 'pgn'])[1:]
	)

openings = pandas.concat(frames)

with open(path, "w") as file:
	for i, opening in openings.iterrows():
		text = opening["name"] + '/'
		board = chess.Board()
		for elem in opening["pgn"].split():
			if '.' in elem: continue
			move = board.parse_san(elem)
			board.push(move)

			text+= chess.SQUARE_NAMES[move.from_square] + '/'
			text+= chess.SQUARE_NAMES[move.to_square] + '/'
		text+= '\n'
		file.write(text)

			
