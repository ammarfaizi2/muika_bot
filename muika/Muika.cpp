// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika.hpp>
#include <muika/ModuleManager.hpp>

#include <unistd.h>

namespace muika {

Muika::Muika(const std::string &token, const std::string &storage_dir):
	bot_(token),
	storage_dir_(storage_dir),
	mod_mgr_(std::make_unique<ModuleManager>(this))
{
}

Muika::~Muika(void) = default;

void Muika::start(void)
{
	TgBot::TgLongPoll longPoll(bot_);


	bot_.getEvents().onAnyMessage([this](TgBot::Message::Ptr msg) {
		handleMessage(msg);
	});

	printf("Bot username: %s\n", bot_.getApi().getMe()->username.c_str());

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

inline
void Muika::handleMessage(TgBot::Message::Ptr &msg)
{
	if (msg->text.empty())
		return;

	mod_mgr_->handleMessage(msg);
}

} /* namespace muika */
