// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__WORKERPOOL_HPP
#define MUIKA__MODULES__M002_JQFTU__WORKERPOOL_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace muika {
namespace modules {
namespace m002_jqftu {

/*
 * A small fixed-size worker pool used by jqftu Sessions to run
 * their card-sending loops without spawning one detached thread
 * per chat.  Each worker initialises a thread-local curl handle
 * (used by Card::generateLatexPng) on entry and tears it down on
 * exit.
 */
class WorkerPool {
public:
	using Task = std::function<void()>;

	explicit WorkerPool(std::size_t nr_workers);
	~WorkerPool(void);

	WorkerPool(const WorkerPool &) = delete;
	WorkerPool &operator=(const WorkerPool &) = delete;

	void submit(Task task);
	void shutdown(void);

	static std::size_t workerCountFromEnv(std::size_t fallback);

private:
	std::vector<std::thread> workers_;
	std::queue<Task> queue_;
	std::mutex mutex_;
	std::condition_variable cv_;
	bool stopping_ = false;

	void workerLoop(void);
};

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__M002_JQFTU__WORKERPOOL_HPP */
