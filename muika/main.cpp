// SPDX-License-Identifier: GPL-2.0-only
#include <muika/Muika.hpp>

#include <cstdlib>
#include <cstdio>

int main(void)
{
	const char *bot_token, *storage_dir;


	bot_token = std::getenv("MUIKA_BOT_TOKEN");
	if (!bot_token) {
		printf("MUIKA_BOT_TOKEN not set\n");
		return 1;
	}

	storage_dir = std::getenv("MUIKA_STORAGE_DIR");
	if (!storage_dir)
		storage_dir = "./storage";

	muika::Muika m(bot_token, storage_dir);
	m.start();

	return 0;
}
