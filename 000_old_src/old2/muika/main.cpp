
#include <cstdio>
#include <cstdlib>
#include "Muika/MuikaBot.hpp"

int main(void)
{
	static const char *bot_token;

	bot_token = getenv("MUIKA_BOT_TOKEN");
	if (!bot_token) {
		fprintf(stderr, "Error: Environment variable MUIKA_BOT_TOKEN is not set.\n");
		return 1;
	}

	Muika::MuikaBot bot(bot_token);
	bot.run();
	return 0;
}
