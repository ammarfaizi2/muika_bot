// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MUIKA_HPP
#define MUIKA__MUIKA_HPP

#include <tgbot/tgbot.h>

#include <string>
#include <memory>

namespace muika {

class ModuleManager;

class Muika {
private:
	TgBot::Bot bot_;
	std::unique_ptr<ModuleManager> mod_mgr_;
	void handleMessage(TgBot::Message::Ptr &msg);
	void initModuleManager(void);
	void installHandlers(void);

public:
	Muika(const std::string &token);
	~Muika(void);

	inline auto &getApi(void) { return bot_.getApi(); }
	inline TgBot::Bot &getBot(void) { return bot_; }
	void start(void);
};

} /* namespace muika */

#endif /* MUIKA__MUIKA_HPP */
