// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/WorkerPool.hpp>
#include <muika/modules/m002_jqftu/Card.hpp>

#include <cstdlib>
#include <utility>

namespace muika {
namespace modules {
namespace m002_jqftu {

WorkerPool::WorkerPool(std::size_t nr_workers)
{
	if (nr_workers == 0)
		nr_workers = 1;

	workers_.reserve(nr_workers);
	for (std::size_t i = 0; i < nr_workers; i++)
		workers_.emplace_back([this]() { workerLoop(); });
}

WorkerPool::~WorkerPool(void)
{
	shutdown();
}

void WorkerPool::submit(Task task)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (stopping_)
			return;
		queue_.push(std::move(task));
	}
	cv_.notify_one();
}

void WorkerPool::shutdown(void)
{
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (stopping_)
			return;
		stopping_ = true;
	}
	cv_.notify_all();

	for (auto &t : workers_) {
		if (t.joinable())
			t.join();
	}
	workers_.clear();
}

void WorkerPool::workerLoop(void)
{
	Card::threadInit();

	while (true) {
		Task task;
		{
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait(lock, [this]() {
				return stopping_ || !queue_.empty();
			});

			if (stopping_ && queue_.empty())
				break;

			task = std::move(queue_.front());
			queue_.pop();
		}

		try {
			task();
		} catch (...) {
			/* Swallow: tasks must report their own errors. */
		}
	}

	Card::threadFree();
}

// static
std::size_t WorkerPool::workerCountFromEnv(std::size_t fallback)
{
	const char *env = std::getenv("MUIKA_JQFTU_WORKERS");
	if (!env || !*env)
		return fallback;

	char *end = nullptr;
	long v = std::strtol(env, &end, 10);
	if (!end || *end || v <= 0)
		return fallback;

	return static_cast<std::size_t>(v);
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
