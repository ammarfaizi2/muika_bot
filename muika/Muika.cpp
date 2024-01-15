// SPDX-License-Identifier: GPL-2.0-only

#include "Muika.hpp"
#include "Jqftu.hpp"

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
	std::string &text = msg->text;

	if (text.empty())
		return;

	if (text.length() > 1) {
		if (text[0] == '/' || text[0] == '!' || text[0] == '.')
			handleCmdMessage(msg);
	}
}

static bool is_space_or_null(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\0';
}

inline void Muika::handleCmdMessage(TgBot::Message::Ptr &msg)
{
	const char *txt = msg->text.c_str() + 1;

	if (msg->text.length() >= 6) {
		if (strncmp(txt, "jqftu", 5) == 0 && is_space_or_null(txt[5])) {
			Jqftu jqftu(*this, msg);
			jqftu.execute();
		}
	}
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
