# SPDX-License-Identifier: GPL-2.0

from aa_tozai_line import TozaiLine
from aa_yamanote_line import YamanoteLine
from aa_keikyu_line import KeikyuLine
import asyncio
import os

async def main():
	os.makedirs("./output", exist_ok=True)

	lines = [
		KeikyuLine(),
		TozaiLine(),
		YamanoteLine()
	]

	for line in lines:
		line.set_save_path(f"./output/{line.line_name}")

	tasks = [line.scrape_station_list() for line in lines]
	await asyncio.gather(*tasks)

	for line in lines:
		line.clean_up_and_print()

	tasks = [line.scrape_all_stations() for line in lines]
	await asyncio.gather(*tasks)

if __name__ == "__main__":
	asyncio.run(main())
