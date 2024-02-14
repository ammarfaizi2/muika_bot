# SPDX-License-Identifier: GPL-2.0

from base_scrapper import BaseJqftuLine

class KeikyuLine(BaseJqftuLine):
	def __init__(self):
		super().__init__()
		self.set_line_name("Keikyu")

		#
		# - KK01 to KK11
		# - KK18 to KK20
		# - KK27 to KK64
		#
		self.add_url("https://en.wikipedia.org/wiki/Keiky%C5%AB_Main_Line")

		#
		# - KK11 (transit point with KK01 to KK11)
		# - KK12 to KK17
		#
		self.add_url("https://en.wikipedia.org/wiki/Keiky%C5%AB_Airport_Line")

		#
		# - KK20 (transit point with KK01 to KK11)
		# - KK21 to KK26
		#
		self.add_url("https://en.wikipedia.org/wiki/Keiky%C5%AB_Daishi_Line")

		#
		# - KK61 (transit point with KK27 to KK64)
		# - KK65 to KK72
		#
		self.add_url("https://en.wikipedia.org/wiki/Keiky%C5%AB_Kurihama_Line")
