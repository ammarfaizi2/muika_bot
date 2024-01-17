// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__COMMAND_HPP
#define MUIKA__COMMAND_HPP

#include "entry.hpp"
#include <vector>
#include <string>

namespace muika {
namespace modules {
namespace jqftu {

class Command {
private:
	Muika &m_;
	TgBot::Message::Ptr &msg_;
	std::vector<std::string> args_;
	void parseArgs(void);
	void showHelp(void);
	void start(void);

public:
	Command(Muika &m, TgBot::Message::Ptr &msg);
	~Command(void) = default;
	void execute(void);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif
