# SPDX-License-Identifier: GPL-2.0

from html2image import Html2Image
from bs4 import BeautifulSoup as bs
from bs4.element import Tag
from pypdl import Downloader
import os, json
import httpx
import pykakasi
from uuid import uuid4
import shutil
from pySmartDL import SmartDL
from functools import partial, wraps
from typing import Callable
import asyncio
import httpx
import re

client = httpx.AsyncClient(headers={
	'accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7',
	'accept-language': 'en-US,en;q=0.9,id;q=0.8,mt;q=0.7',
	'cache-control': 'max-age=0',
	'sec-ch-ua': '"Not A(Brand";v="99", "Microsoft Edge";v="121", "Chromium";v="121"',
	'sec-ch-ua-mobile': '?0',
	'sec-ch-ua-platform': '"Windows"',
	'sec-fetch-dest': 'document',
	'sec-fetch-mode': 'navigate',
	'sec-fetch-site': 'same-origin',
	'sec-fetch-user': '?1',
	'upgrade-insecure-requests': '1',
	'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36 Edg/121.0.0.0',
})


async def http_get_text(url: str) -> str:
	return (await client.get(url)).text


#
# For every <tr> tag in the station list table, there exists a BaseJqftuRawStation
# object that will extract the station name, romaji, and kanji from the <tr> tag
# and store it in a dictionary.
#
class BaseJqftuRawStation:
	def __init__(self, html: str):
		self.html = html
		self.is_valid = False


	def parse_wiki_url(self) -> str:
		u = self.html.find('a', attrs={'title': re.compile('Station')})
		if u is None:
			raise Exception("Cannot parse the wiki url")

		return f"https://en.wikipedia.org{u['href']}"


	def parse_number(self) -> str:
		u = self.html.find('span', attrs={'style': re.compile('vertical-align:middle')})
		if u is None:
			raise Exception("Cannot parse the station number (r1)")

		u = u.find_all('span')
		if len(u) < 2:
			raise Exception("Cannot parse the station number (r2)")

		_T = u[0].text
		_N = u[1].text
		return f"{_T}{_N.zfill(2)}"


	def parse_romaji(self) -> str:
		u = self.html.find('a', attrs={'title': re.compile('Station')})
		return u['title'].split(' Station')[0].lower()


	def parse_kanji(self) -> str:
		td = None
		tr_attrs_variants = [
			{'style': re.compile('right')},
			{'align': 'right'},
			{'align': 'center'}
		]

		for i in tr_attrs_variants:
			td = self.html.find('td', attrs=i)
			if td is not None:
				break

		if td is None:
			raise Exception("Cannot parse the kanji")

		kanji = td.find_previous_sibling('td')

		# Remove the <sup> tag if exists.
		sup = kanji.find('sup')
		if sup:
			sup.extract()

		return kanji.get_text(strip=True)


	def parse(self):
		try:
			self.n = self.parse_number()
			self.kanji = self.parse_kanji()
			self.romaji = self.parse_romaji()
			self.wiki_url = self.parse_wiki_url()

			#
			# Success to parse all fields, very good!
			#
			self.is_valid = True
		except BaseException as e:
			print("BaseJqftuRawStation: Failed to parse station:", e)


#
# For every station, there exists a BaseJqftuStation object that will store the
# station name, romaji, kanji, hiragana, katakana, wiki url, and photos.
#
# BaseJqftuStation always needs BaseJqftuRawStation object to be initialized.
#
class BaseJqftuStation:
	def __init__(self, save_path, raw_station: BaseJqftuRawStation):
		#
		# For JSON end result.
		#
		self.n = raw_station.n
		self.kanji = raw_station.kanji
		self.romaji = raw_station.romaji
		self.hiragana = ""
		self.katakana = ""
		self.q_img = f"{self.n}.png"
		self.photos = []
		self.wiki_url = raw_station.wiki_url

		#
		# For internal use.
		#
		self.save_path = save_path
		self.photos_url = []
		self.st_num_html = []
		self.is_valid = False


	async def scrape(self):
		self.html = bs(await http_get_text(self.wiki_url), 'html.parser')


	#
	# The JSON result will be taken from to_dict() method.
	#
	def to_dict(self):
		return {
			"n": self.n,
			"kanji": self.kanji,
			"romaji": self.romaji,
			"hiragana": self.hiragana,
			"katakana": self.katakana,
			"q_img": self.q_img,
			"photos": self.photos,
			"wiki_url": self.wiki_url
		}


	def parse_station_number_images(self) -> list:
		#
		# TODO(@sunda005): Collect the station number images (HTML string)
		#                  from the wiki page.
		#
		return []


	def parse_photos_url(self) -> list:
		#
		# TODO(@sunda005): Collect the photos URL from the wiki page.
		#
		return []


	def construct_q_img(self):
		#
		# TODO(@sunda005): Construct the q_img from the station number
		#                  HTML string (self.st_num_html).
		#
		#                  Then save the constructed image to:
		#                  f"{self.save_path}/{self.q_img}".
		#
		pass


	def download_photos(self):
		#
		# TODO(@sunda005):
		#
		#  - mkdir f"{self.save_path}/photos" (if not exists).
		#
		#  - mkdir f"{self.save_path}/photos/{self.n}".
		#
		#  - Download all photos in self.photos_url, save it to:
		#    f"{self.save_path}/photos/{self.n}/{uuid4()}.jpg".
		#
		#  - For each saved photo, append f"{self.n}/{uuid4()}.jpg"
		#    to self.photos.
		#
		pass


	def construct_kana(self):
		k = pykakasi.kakasi()
		self.hiragana = self.katakana = ""
		for i in k.convert(self.kanji):
			self.hiragana += i['hira']
			self.katakana += i['kana']


	def parse(self):
		try:
			#
			# TODO(@ammarfaizi2): Revisit this, make sure everything
			#                     is properly integrated.
			#
			self.st_num_html = self.parse_station_number_images()
			self.photos_url = self.parse_photos_url()
		except BaseException as e:
			print("BaseJqftuStation: Failed to parse station:", e)


	def save(self):
		try:
			self.construct_kana()
			self.construct_q_img()
			self.download_photos()

			#
			# Success to save the station, very good!
			#
			self.is_valid = True
		except BaseException as e:
			print("BaseJqftuStation: Failed to save station:", e)


#
# For every line, there exists a BaseJqftuLine object. Note that a single line
# can have multiple URLs (e.g. Keikyu and Keisei lines).
#
# BaseJqftuLine collects all the station list from the URLs, and then construct
# a list of BaseJqftuRawStation objects, remove the duplicates, and sort them
# by the station number.
#
# Once the station list is ready, BaseJqftuLine will then construct a list of
# BaseJqftuStation objects. Each BaseJqftuStation object scrapes the station
# information and photos from the wiki URL.
#
class BaseJqftuLine:
	def __init__(self):
		self.line_urls = []
		self.station_list = []
		self.line_name = ""
		self.save_path = "./"


	def set_line_name(self, line_name: str) -> None:
		self.line_name = line_name

		# Add space padding to the line name for better logging.
		self.ln = self.line_name.ljust(10, ' ')
		self.ln = f"[{self.ln}]: "


	def add_url(self, line_url: str) -> None:
		self.line_urls.append(line_url)


	def set_save_path(self, save_path: str) -> None:
		os.makedirs(save_path, exist_ok=True)
		self.save_path = save_path


	#
	# Pass a <tr> tag in the station list table, and return a
	# BaseJqftuRawStation object.
	#
	def parse_tr_station(self, html: Tag) -> BaseJqftuRawStation:
		st = BaseJqftuRawStation(html)
		st.parse()
		return st


	async def _scrape_station_list(self, line_url: str):
		html = bs(await http_get_text(line_url), 'html.parser')
		html = html.find('table', class_=re.compile('wikitable', re.IGNORECASE))
		html = html.find('tbody')
		ret = []

		for i in html.find_all('tr')[1:-1]:

			if len(i.find_all('td')) <= 1:
				# Not a station row.
				continue

			st = self.parse_tr_station(i)
			if st.is_valid:
				ret.append(st)

		return ret


	def clean_up_station_list(self):
		# Remove duplicates. Use the station number as the key.
		self.station_list = list({i.n: i for i in self.station_list}.values())

		# Sort by the station number string.
		self.station_list.sort(key=lambda x: x.n)


	#
	# Scrape the station list from each URL and merge them into a single
	# list.
	#
	async def scrape_station_list(self):
		for i, url in enumerate(self.line_urls):
			print(f"{self.ln}Scrapping [{i+1}/{len(self.line_urls)}]: {url}")
			st_list = await self._scrape_station_list(url)
			self.station_list.extend(st_list)


	def clean_up_and_print(self):
		self.clean_up_station_list()
		print(f"{self.ln}Number of stations: {len(self.station_list)}")


	async def scrape_station(self, station: BaseJqftuRawStation) -> BaseJqftuStation:
		st = BaseJqftuStation(self.line_name, station)
		await st.scrape()
		st.parse()
		st.save()
		return st


	def store_json(self, ret: list):
		j = []
		for i in ret:
			if i.is_valid:
				j.append(i.to_dict())

		fn = self.line_name.lower().replace(' ', '_')
		fn = f"{self.save_path}/{fn}.json"
		with open(fn, 'w', encoding='utf-8') as f:
			json.dump(j, f, indent=4, ensure_ascii=False)

		print(f"{self.ln}Saved to {fn}")


	async def scrape_all_stations(self):
		ret = []
		for i in self.station_list:
			print(f"{self.ln}Scrapping: {i.wiki_url}")
			st = await self.scrape_station(i)
			ret.append(st)
			self.store_json(ret)
