// SPDX-License-Identifier: GPL-2.0-only

#include "entry.hpp"
#include "Card.hpp"
#include "Deck.hpp"
#include "Session.hpp"
#include "internal.hpp"
#include "Command.hpp"

namespace muika {
namespace modules {
namespace jqftu {

std::unordered_map<int64_t, Session *> sessions;
std::mutex sessions_mutex;

static bool is_space_or_null(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\0';
}

static module_ret_t handle_jqftu_command(muika::Muika &m,
					 TgBot::Message::Ptr &msg)
{
	const char *txt = msg->text.c_str();

	if (msg->text.length() < 6)
		return MOD_ENTRY_CONTINUE;

	/*
	 * The command must start with '/' or '!' or '.'.
	 */
	if (txt[0] != '/' && txt[0] != '!' && txt[0] != '.')
		return MOD_ENTRY_CONTINUE;

	if (memcmp(txt + 1, "jqftu", 5))
		return MOD_ENTRY_CONTINUE;

	if (!is_space_or_null(txt[6]))
		return MOD_ENTRY_CONTINUE;

	Command c(m, msg);
	c.execute();
	return MOD_ENTRY_STOP;
}

module_ret_t entry(muika::Muika &m, TgBot::Message::Ptr &msg)
{
	return handle_jqftu_command(m, msg);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
