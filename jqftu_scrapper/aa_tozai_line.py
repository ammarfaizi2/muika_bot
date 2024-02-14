# SPDX-License-Identifier: GPL-2.0

from base_scrapper import BaseJqftuLine

class TozaiLine(BaseJqftuLine):
	def __init__(self):
		super().__init__()
		self.set_line_name("Tozai")
		self.add_url("https://en.wikipedia.org/wiki/Tokyo_Metro_Tozai_Line")
