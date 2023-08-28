import time
import chess

from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.action_chains import ActionChains

class Lichess_player:
	time_steps = [180, 165, 150, 135, 120, 105, 90, 75, 60, 45, 40, 35, 30, 25, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1.5, 1, .75, .5, .25, 0]
	incr_steps = [180, 150, 120, 90, 60, 45, 40, 35, 30, 25, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0]

	def get_steps(self):
		time_choice = self.driver.find_element(By.XPATH, "//div[@class='time-choice range']/input")
		incr_choice = self.driver.find_element(By.XPATH, "//div[@class='increment-choice range']/input")

		time_choice.send_keys(Keys.END)
		v = 1
		while v != '0': # Move left for the desired number of steps
			action = ActionChains(self.driver)
			action.send_keys(Keys.ARROW_LEFT)
			action.perform()
			print(v := self.driver.find_element(By.XPATH, "//div[@class='time-choice range']/span").text, end=', ')
		print('')

		incr_choice.send_keys(Keys.END)
		v = 1
		while v != '0': # Move left for the desired number of steps
			action = ActionChains(self.driver)
			action.send_keys(Keys.ARROW_LEFT)
			action.perform()
			print(v := self.driver.find_element(By.XPATH, "//div[@class='increment-choice range']/span").text, end=', ')
		print('')

	def await_move(self):
		request = lambda: self.driver.find_element(By.TAG_NAME, "l4x").find_elements(By.TAG_NAME, "kwdb")
		n_moves = len(request())
		while len((moves := request())) == n_moves: time.sleep(.2)
		san = moves[-1].text
		mv = self.board.push_san(san)
		return mv.uci()

	def __init__(self, col, time_=5, incr=3):
		self.col = col

		self.driver = webdriver.Firefox()
		# self.driver.get("https://lichess.org")

		# self.driver.find_element(By.XPATH, "//button[text()='Neue Partie']").click()

		# time_choice = self.driver.find_element(By.XPATH, "//div[@class='time-choice range']/input")
		# incr_choice = self.driver.find_element(By.XPATH, "//div[@class='increment-choice range']/input")

		# time_choice.send_keys(Keys.END)
		# for _ in range(self.time_steps.index(time_)): time_choice.send_keys(Keys.ARROW_LEFT)

		# incr_choice.send_keys(Keys.END)
		# for _ in range(self.incr_steps.index(incr)): incr_choice.send_keys(Keys.ARROW_LEFT)

		# value = "white" if col else "black"
		# self.driver.find_element(By.XPATH, f"//button[@value='{value}']").click()

		self.driver.get("https://lichess.org/analysis")

		while not (container := self.driver.find_elements(By.TAG_NAME, "cg-container")): time.sleep(1)

		print(f"found game {time_}+{incr}")

		self.canvas = int(container[0].get_attribute("style").split(' ')[-1][:-3])
		self.square = self.canvas / 8

		self.board = chess.Board()

	def click_square(self, x, y):
		print(f"{x},{y} >")
		container = self.driver.find_element(By.TAG_NAME, "cg-container")
		action = ActionChains(self.driver)
		if self.col:
			action.move_to_element_with_offset(container, self.square*x + self.square/2 - self.canvas/2, self.square*(7-y) + self.square/2 - self.canvas/2)
		else:
			action.move_to_element_with_offset(container, self.square*x + self.square/2 - self.canvas/2, self.square*y + self.square/2 - self.canvas/2)
		action.click()
		action.perform()
		time.sleep(.5)

	def do_move(self, uci):
		mv = chess.Move.from_uci(uci)
		self.board.push(mv)

		pos = mv.from_square % 8, mv.from_square // 8
		self.click_square(*pos)

		pos = mv.to_square % 8, mv.to_square // 8
		self.click_square(*pos)

	def quit(self):
		self.driver.find_element(By.CLASS_NAME, "resign").click()
		time.sleep(.5)
		self.driver.find_element(By.CLASS_NAME, "yes").click()
		time.sleep(2)
		self.driver.quit()


lip = Lichess_player(col=1)

input(">")

lip.do_move("e2e3")

print(lip.await_move())

input(">")

lip.quit()

