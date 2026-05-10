// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA__MODULES__JQFTU__WORKER_HPP
#define MUIKA__MUIKA__MODULES__JQFTU__WORKER_HPP

#include <muika/Muika/Modules/Jqftu/ModJqftu.hpp>
#include <muika/Muika/Modules/Jqftu/Session.hpp>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>
#include <string>

namespace Muika {
namespace Modules {
namespace Jqftu {

class ModJqftu;
class Session;

struct WaitThreads {
	std::mutex mtx;
	std::condition_variable cv;
	uint32_t nr_ready;

	inline WaitThreads(void): nr_ready(0) {}

	inline void ready(void)
	{
		std::lock_guard<std::mutex> lk(mtx);
		nr_ready++;
		cv.notify_all();
	}

	inline void wait(uint32_t n)
	{
		std::unique_lock<std::mutex> lk(mtx);
		while (nr_ready < n)
			cv.wait(lk);
	}
};

enum {
	JQFTU_MSG_TYPE_INVALID = -1,
	JQFTU_MSG_TYPE_CMD = 0,
	JQFTU_MSG_TYPE_ANSWER = 1,
};

struct Msg {
	uint8_t type;
	std::shared_ptr<Session> sess = nullptr;
	std::vector<std::string> cmd_args;
	TgBot::Message::Ptr orig;

	inline Msg(TgBot::Message::Ptr o, std::shared_ptr<Session> s = nullptr):
		type(JQFTU_MSG_TYPE_INVALID),
		sess(std::move(s)),
		cmd_args(),
		orig(o)
	{
	}

	inline void setCmd(std::vector<std::string> args)
	{
		type = JQFTU_MSG_TYPE_CMD;
		cmd_args = std::move(args);
	}

	inline void setAnswer(void)
	{
		type = JQFTU_MSG_TYPE_ANSWER;
	}
};

class Worker {
public:
	Worker(ModJqftu *mj, uint32_t tid);
	~Worker(void);
	void start(WaitThreads *wt);
	void stop(void);
private:
	std::thread thread_;
	ModJqftu *mj_;
	uint32_t tid_;

	void run(WaitThreads *wt);
	void popAndProcessMsg(std::unique_lock<std::mutex> &lk);
	void processMsg(std::unique_ptr<Msg> &msg);
	void handleCmd(std::unique_ptr<Msg> &msg);
	void handleCmdStop(std::unique_ptr<Msg> &msg);
	void handleCmdStart(std::unique_ptr<Msg> &msg);
	void handleCmdHelp(std::unique_ptr<Msg> &msg);
	void handleCmdUnknown(std::unique_ptr<Msg> &msg);
};

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */

#endif
