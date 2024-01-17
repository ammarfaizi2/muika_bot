// SPDX-License-Identifier: GPL-2.0-only

#include "Muika.hpp"
#include "ModuleManager.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace muika {

Muika::Muika(const std::string &token):
	bot_(token)
{
	srand(time(NULL));
}

Muika::~Muika(void)
{
}

inline void Muika::installHandlers(void)
{
	bot_.getEvents().onAnyMessage([this](TgBot::Message::Ptr msg) {
		handleMessage(msg);
	});
}

inline void Muika::handleMessage(TgBot::Message::Ptr &msg)
{
	ModuleManager::invokeModules(*this, msg);
}

void Muika::start(void)
{
	TgBot::TgLongPoll longPoll(bot_);

	installHandlers();
	printf("Bot username: %s\n", bot_.getApi().getMe()->username.c_str());
	while (true)
		longPoll.start();
}

} /* namespace muika */
