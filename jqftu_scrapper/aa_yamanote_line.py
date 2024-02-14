# SPDX-License-Identifier: GPL-2.0

from base_scrapper import BaseJqftuLine, BaseJqftuRawStation, BaseJqftuStation

g_outsuka_html = """
<div class="ib-station-name">
	<div class="fn org" style="display:inline">
		<span style="display:inline-block; position:relative; vertical-align:middle; border:5px solid #9ACD32; border-radius:10%; margin:1px; height:40px; width:40px; background:White; text-align:center; font-weight:Bold;">
			<span style="width:40px; height:15px; text-align:center; line-height:15px; font-size:15px; color:black; position:absolute; top:1px; left:0;">JY</span><span style="width:40px; height:24px;; text-align:center; line-height:24px; font-size:24px; color:black; position:absolute; bottom:1px; left:0;">12</span>
		</span>
		<span style="display:inline-block; position:relative; vertical-align:middle; background:White; text-align:center; font-weight:Bold;">
			<img style="width:50px; height:50px;" src="https://upload.wikimedia.org/wikipedia/commons/9/9b/SA-23_station_number.png"/>
		</span>
		<div class="kanji" style="font-size:70px">大塚</div>
	</div>
</div>
"""

class SpecialStation(BaseJqftuStation):
	async def construct_q_img(self):
		if (self.n == "JY12"):
			await self.st_outsuka()
			return

		await super().construct_q_img()


	async def st_outsuka(self):
		zoom_level = 450
		html = g_outsuka_html
		await self._take_screenshot(html=html, zoom_level=zoom_level)


class YamanoteLine(BaseJqftuLine):
	def __init__(self):
		super().__init__()
		self.set_line_name("Yamanote")
		self.add_url("https://en.wikipedia.org/wiki/Yamanote_Line")


	async def scrape_station(self, station: BaseJqftuRawStation) -> BaseJqftuStation:
		if station.n == "JY12":
			return await self.handle_special_case(station)
		else:
			return await super().scrape_station(station)


	async def handle_special_case(self, station: BaseJqftuRawStation) -> BaseJqftuStation:
		st = SpecialStation(self.save_path, station)
		await st.scrape()
		st.parse()
		await st.save()
		return st
