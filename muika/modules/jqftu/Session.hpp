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
	std::string full_name_;
	std::string username_;
	inline Score(uint32_t point, std::string full_name, std::string username):
		point_(point),
		full_name_(full_name),
		username_(username)
	{
	}
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
	uint32_t timeout_ = 1800;
	uint32_t next_delay_ = 5;
	volatile bool should_stop_ = false;
	std::atomic<int> ref_count_;
	uint64_t last_msg_id_ = 0;

	void worker(void);
	bool sendCard(void);
	void sendFailMessage(const std::string &msg);
	void sendFinishMessage(void);

public:
	Session(Muika &m, int64_t chat_id, const std::string &deck_name);
	~Session(void);
	void start(void);
	void stop(void);
	void answer(const TgBot::Message::Ptr &msg);
	void setTimeout(uint32_t timeout, bool skip = true);
	void setNextDelay(uint32_t next_delay, bool skip = true);

	static Session *getSession(int64_t chat_id);
	static void putSession(Session *sess);
	static Session *createSession(Muika &m, int64_t chat_id, const std::string &deck_name);
	static void deleteSession(int64_t chat_id);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__SESSION_HPP */
