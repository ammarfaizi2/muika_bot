// SPDX-License-Identifier: GPL-2.0-only

#include "Muika.hpp"
#include <cstdlib>
#include <cstdio>

int main(void)
{
	const char *bot_token = std::getenv("MUIKA_BOT_TOKEN");
	if (!bot_token) {
		printf("MUIKA_BOT_TOKEN not set\n");
		return 1;
	}

	muika::Muika m(bot_token);
	m.start();

	return 0;
}
