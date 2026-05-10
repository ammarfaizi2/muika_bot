// SPDX-License-Identifier: GPL-2.0-only
#include <mtgbot/Bot.hpp>

static std::unique_ptr<mtgbot::Bot> create_mtgbot(void)
{
	const char *token = getenv("MUIKA_TGBOT_TOKEN");
	if (!token) {
		fprintf(stderr, "MUIKA_TGBOT_TOKEN environment variable is not set\n");
		return nullptr;
	}

	mtgbot::BotConfig cfg;
	cfg.token = token;
	cfg.storage_path = "./storage/mtgbot";
	return std::make_unique<mtgbot::Bot>(cfg);
}

int main(void)
{
	std::unique_ptr<mtgbot::Bot> bot = create_mtgbot();
	if (!bot)
		return 1;

	bot->run();
	return 0;
}
