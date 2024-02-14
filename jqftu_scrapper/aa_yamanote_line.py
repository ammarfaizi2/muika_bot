# SPDX-License-Identifier: GPL-2.0

from base_scrapper import BaseJqftuLine

class YamanoteLine(BaseJqftuLine):
	def __init__(self):
		super().__init__()
		self.set_line_name("Yamanote")
		self.add_url("https://en.wikipedia.org/wiki/Yamanote_Line")
