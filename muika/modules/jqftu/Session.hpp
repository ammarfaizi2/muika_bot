// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__SESSION_HPP
#define MUIKA__MODULES__JQFTU__SESSION_HPP

#include "muika/Muika.hpp"
#include "muika/modules/jqftu/Deck.hpp"
#include "muika/modules/jqftu/Card.hpp"

#include <mutex>
#include <thread>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

namespace muika {
namespace modules {
namespace jqftu {

struct Score {
	uint32_t point_;
	inline Score(uint32_t point): point_(point) {}
};

class Session {
private:
	Muika &m_;
	int64_t chat_id_;
	std::thread worker_;
	std::mutex mutex_;
	std::condition_variable	cond_;
	std::unique_ptr<Deck> deck_;
	Card *current_card_ = nullptr;
	std::unordered_map<int64_t, Score> scores_;
	uint32_t timeout_ = 300;
	uint32_t next_delay_ = 5;
	volatile bool should_stop_ = false;
	std::atomic<int> ref_count_;

	void worker(void);

public:
	Session(Muika &m, int64_t chat_id, const std::string &deck_name);
	~Session(void);
	void start(void);
	void stop(void);
	void answer(const TgBot::Message::Ptr &msg);

	static Session *getSession(int64_t chat_id);
	static void putSession(Session *sess);
	static Session *createSession(Muika &m, int64_t chat_id, const std::string &deck_name);
	static void deleteSession(int64_t chat_id);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__SESSION_HPP */
