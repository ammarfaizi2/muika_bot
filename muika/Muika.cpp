// SPDX-License-Identifier: GPL-2.0-only

#include "muika/Muika.hpp"
#include "muika/ModuleManager.hpp"
#include "muika/modules/jqftu/entry.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <stdexcept>

#include <unistd.h>

namespace muika {

static struct Module muika_modules_array[] = {
	{
		.init  = &muika::modules::jqftu::init,
		.entry = &muika::modules::jqftu::entry,
		.free  = &muika::modules::jqftu::free,
	},
};

Muika::Muika(const std::string &token):
	bot_(token)
{
	srand(time(NULL));
}

Muika::~Muika(void) = default;

inline void Muika::initModuleManager(void)
{
	const size_t nr_modules = sizeof(muika_modules_array) / sizeof(muika_modules_array[0]);

	mod_mgr_ = std::make_unique<ModuleManager>(muika_modules_array, nr_modules);
	if (mod_mgr_->initModules(*this) < 0)
		throw std::runtime_error("Failed to initialize modules");
}

inline void Muika::installHandlers(void)
{
	bot_.getEvents().onAnyMessage([this](TgBot::Message::Ptr msg) {
		handleMessage(msg);
	});
}

inline void Muika::handleMessage(TgBot::Message::Ptr &msg)
{
	try {
		mod_mgr_->invokeModules(*this, msg);
	} catch (std::exception &e) {
		printf("Error: %s\n", e.what());
	} catch (...) {
		printf("Unknown error\n");
	}
}

void Muika::start(void)
{
	TgBot::TgLongPoll longPoll(bot_);

	printf("Bot username: %s\n", bot_.getApi().getMe()->username.c_str());
	initModuleManager();
	installHandlers();

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

} /* namespace muika */
