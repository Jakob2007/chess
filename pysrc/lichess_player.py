import time
import random

from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.common.action_chains import ActionChains

class Lichess_player:
	name = "Lichess"

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

	def setup(self):
		self.driver.find_element(By.XPATH, "//button[text()='Neue Partie']").click()

		time_choice = self.driver.find_element(By.XPATH, "//div[@class='time-choice range']/input")
		incr_choice = self.driver.find_element(By.XPATH, "//div[@class='increment-choice range']/input")

		time_choice.send_keys(Keys.END)
		for _ in range(self.time_steps.index(self.game.li_time)): time_choice.send_keys(Keys.ARROW_LEFT)

		incr_choice.send_keys(Keys.END)
		for _ in range(self.incr_steps.index(self.game.li_incr)): incr_choice.send_keys(Keys.ARROW_LEFT)

		value = "white" if not self.col else "black"
		self.driver.find_element(By.XPATH, f"//button[@value='{value}']").click()

	def __init__(self, game, col):
		self.col = col
		self.game = game

		opt = webdriver.FirefoxOptions()
		#opt.add_argument('--headless')
		self.driver = webdriver.Firefox(options=opt)
		self.driver.get("https://lichess.org")

		self.setup()

		while not (name := self.driver.find_elements(By.XPATH, "//div[contains(@class, 'user-link') and contains(@class, 'online')]")): time.sleep(1)
		while not (container := self.driver.find_elements(By.TAG_NAME, "cg-container")): time.sleep(1)

		self.name = name[0].text
		print(f"found game {game.li_time}+{game.li_incr} vs {self.name}")

		self.canvas = int(container[0].get_attribute("style").split(' ')[-1][:-3])
		self.square = self.canvas / 8

	def await_move(self):
		while not self.driver.find_elements(By.TAG_NAME, "l4x"): time.sleep(.1)
		
		request = lambda: self.driver.find_element(By.TAG_NAME, "l4x").find_elements(By.TAG_NAME, "kwdb")
		while self.game.running and len(moves := request()) % 2 != self.col:
			if not self.driver.find_elements(By.XPATH, "//div[contains(@class, 'user-link') and contains(@class, 'online')]"):
				return None
			if self.driver.find_elements(By.CLASS_NAME, 'time'):
				self.game.info = self.driver.find_element(By.CLASS_NAME, 'time').text.replace(':', ';').replace('\n',' ')
			time.sleep(.2)
		if not self.game.running: return None
		san = moves[-1].text
		mv = self.game.board.parse_san(san)
		return mv

	def click_square(self, x, y):
		container = self.driver.find_element(By.TAG_NAME, "cg-container")
		action = ActionChains(self.driver)
		if not self.col:
			action.move_to_element_with_offset(container, self.square*x + self.square/2 - self.canvas/2, self.square*(7-y) + self.square/2 - self.canvas/2)
		else:
			action.move_to_element_with_offset(container, self.square*(7-x) + self.square/2 - self.canvas/2, self.square*y + self.square/2 - self.canvas/2)
		action.click()
		action.perform()
		time.sleep(random.uniform(.3, .7))

	def push_move(self, mv):
		if not self.driver.find_elements(By.TAG_NAME, "l4x"):
			time.sleep(.1)
			pos = mv.from_square % 8, mv.from_square // 8
			self.click_square(*pos)

			pos = mv.to_square % 8, mv.to_square // 8
			self.click_square(*pos)
		else:
			request = lambda: self.driver.find_element(By.TAG_NAME, "l4x").find_elements(By.TAG_NAME, "kwdb")
			while self.game.running and len(request()) % 2 == self.col:
				time.sleep(.1)
				pos = mv.from_square % 8, mv.from_square // 8
				self.click_square(*pos)

				pos = mv.to_square % 8, mv.to_square // 8
				self.click_square(*pos)

	def quit(self):
		if self.driver.find_elements(By.CLASS_NAME, "resign"):
			self.driver.find_element(By.CLASS_NAME, "resign").click()
			time.sleep(.5)
			if self.driver.find_elements(By.CLASS_NAME, "yes"):
				self.driver.find_element(By.CLASS_NAME, "yes").click()
				time.sleep(2)
		self.driver.quit()

class Stockfish_player(Lichess_player):
	name = "Lichess-Stockfish"

	def setup(self):
		self.driver.find_element(By.XPATH, "//button[text()='Spiele mit dem Computer']").click()

		value = "white" if not self.col else "black"
		self.driver.find_element(By.XPATH, f"//button[@value='{value}']").click()
