// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MUIKA_HPP
#define MUIKA__MUIKA_HPP

#include <tgbot/tgbot.h>
#include <string>

namespace muika {

class Muika {
private:
	TgBot::Bot bot_;
	void installHandlers(void);
	void handleMessage(TgBot::Message::Ptr &msg);
	void handleCmdMessage(TgBot::Message::Ptr &msg);

public:
	Muika(const std::string &token);
	~Muika(void);

	inline TgBot::Bot &getBot(void) { return bot_; }
	void start(void);
};

} /* namespace muika */

#endif /* MUIKA__MUIKA_HPP */
