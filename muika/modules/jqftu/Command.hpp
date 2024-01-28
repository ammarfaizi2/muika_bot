// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__COMMAND_HPP
#define MUIKA__MODULES__JQFTU__COMMAND_HPP

#include "muika/modules/jqftu/SessionMap.hpp"

namespace muika {
namespace modules {
namespace jqftu {

class Command {
private:
	Muika &m_;
	const TgBot::Message::Ptr &msg_;
	std::vector<std::string> args_;
	std::shared_ptr<Session> sess_ = nullptr;
	SessionMap *smap_;
	std::unique_lock<std::mutex> smap_lock_;

	uint64_t sendMsg(const std::string &msg, uint64_t reply_to = (uint64_t)-1);
	void sendInvalidCommandResponse(void);
	__must_hold(&smap_->mutex_) bool __loadSession(void);
	bool loadSession(void);
	bool checkAnswerAttempt(void);
	bool parseCmd(void);
	void parseArgs(void);
	void scoreboard(void);
	void points(void);
	void start(void);
	void stop(void);
	void help(void);

public:
	Command(Muika &m, const TgBot::Message::Ptr &msg, SessionMap *smap);
	~Command(void) = default;
	bool execute(void);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__COMMAND_HPP */
