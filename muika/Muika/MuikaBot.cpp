// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/MuikaBot.hpp>
#include <muika/Muika/ModuleManager.hpp>

#include <cstring>
#include <tgbot/tgbot.h>

#include <unistd.h>

namespace Muika {

MuikaBot::MuikaBot(const char *token) noexcept
{
	strncpy(token_, token, sizeof(token_) - 1);
	token_[sizeof(token_) - 1] = '\0';
}

MuikaBot::~MuikaBot(void) = default;

inline void MuikaBot::installHandlers(void)
{
	bot_->getEvents().onAnyMessage([this](TgBot::Message::Ptr msg) {
		mod_mgr_->invokeModules(msg);
	});
}

void MuikaBot::run(void)
{
	bot_ = std::make_unique<TgBot::Bot>(token_);
	pr_info("Bot username: @%s", bot_->getApi().getMe()->username.c_str());

	installHandlers();
	mod_mgr_ = std::make_unique<ModuleManager>(this);

	pr_info("Start polling...");
	TgBot::TgLongPoll longPoll(*bot_);
	while (true) {
		try {
			while (true)
				longPoll.start();
		} catch (std::exception &e) {
			printf("Error: %s\n", e.what());
		} catch (...) {
			printf("Unknown error\n");
		}

		sleep(3);
	}
}

} /* namespace Muika */
