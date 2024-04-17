# SPDX-License-Identifier: GPL-2.0

from base_scrapper import BaseJqftuLine, BaseJqftuRawStation, BaseJqftuStation


class SpecialStation(BaseJqftuStation):
	#
	# The JSON result will be taken from to_dict() method.
	#
	def to_dict(self):
		r = super().to_dict()
		if self.n == "KK16":
			r["alt"] = [
				"haneda kuukou dai 3 taaminaru",
				"haneda airport terminal 3",
				"haneda kuukou terminal 3"
			]
		elif self.n == "KK17":
			r["alt"] = [
				"haneda kuukou dai 1 dai 2 taaminaru",
				"haneda airport terminal 1.2",
				"haneda airport terminal 1",
				"haneda airport terminal 2",
				"haneda kuukou terminal 1.2",
				"haneda kuukou terminal 1",
				"haneda kuukou terminal 2"
			]

		return r


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


	async def scrape_station(self, station: BaseJqftuRawStation) -> BaseJqftuStation:
		return await self.handle_special_case(station)


	async def handle_special_case(self, station: BaseJqftuRawStation) -> BaseJqftuStation:
		st = SpecialStation(self.save_path, station)
		await st.scrape()
		st.parse()
		await st.save()
		return st
