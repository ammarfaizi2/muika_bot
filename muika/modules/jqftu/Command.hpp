// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__COMMAND_HPP
#define MUIKA__MODULES__JQFTU__COMMAND_HPP

#include <muika/Muika.hpp>
#include <muika/modules/jqftu/SessionMap.hpp>

#include <vector>

namespace muika {
namespace modules {
namespace jqftu {

class Command {
private:
	Muika &m_;
	TgBot::Message::Ptr &msg_;
	SessionMap &smap_;

	std::string cmd_;
	std::string sub_cmd_;
	std::vector<std::string> args_;

	uint64_t sendMsg(const std::string &msg, uint64_t reply_to = (uint64_t)-1);

	bool parseCommand(void);
	bool handleCommand(void);

	bool handleCmdStart(void);
	bool handleCmdStop(void);
	bool handleCmdHelp(void);
	bool handleCmdPoints(void);
	bool handleCmdScoreboard(void);
	bool handleCmdSetTimeout(void);
	bool handleCmdSetNextCardDelay(void);

	bool handleUnknownCommand(void);

public:
	Command(Muika &m, TgBot::Message::Ptr &msg, SessionMap &smap);
	~Command(void);
	bool handle(void);
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__COMMAND_HPP */
