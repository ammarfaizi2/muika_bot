// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA__MODULES__JQFTU__MODJQFTU_HPP
#define MUIKA__MUIKA__MODULES__JQFTU__MODJQFTU_HPP

#include <muika/Muika/Module.hpp>
#include <muika/Muika/Modules/Jqftu/Worker.hpp>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>

namespace Muika {
namespace Modules {
namespace Jqftu {

class Msg;

class ModJqftu: public Muika::Module {
public:
	ModJqftu(MuikaBot *mb);
	~ModJqftu(void) override;
	int init(void) override;
	void shutdown(void) override;
	int invoke(TgBot::Message::Ptr &msg) override;

	inline void waitAndSleep(std::unique_lock<std::mutex> &lk)
	{
		nr_sleeping_++;
		cv_.wait(lk);
		nr_sleeping_--;
	}

private:
	std::vector<std::unique_ptr<Worker>> workers_;
	std::queue<std::unique_ptr<Msg>> msg_queue_;
	std::condition_variable cv_;
	std::mutex cv_mtx_;
	uint32_t nr_sleeping_ = 0;
	bool should_stop_ = false;

	std::unique_ptr<Msg> constructMsg(const TgBot::Message::Ptr &msg);
	friend class Worker;
};

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */

#endif
