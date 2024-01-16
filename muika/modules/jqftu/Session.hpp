// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__SESSION_HPP
#define MUIKA__SESSION_HPP

#include "../../ModuleManager.hpp"
#include "Card.hpp"
#include "Deck.hpp"

#include <cstdint>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>

namespace muika {
namespace modules {
namespace jqftu {

class Session {
private:
	Muika &m_;
	int64_t chat_id_;
	std::mutex mutex_;
	std::thread worker_;
	Deck deck_;
	std::condition_variable cv_;
	Card *current_card_ = nullptr;
	uint32_t last_msg_id_ = 0;

	void worker(void);
	void drawAndSendCardLocked(void);
	void handleCorrectAnswer(TgBot::Message::Ptr &msg);
public:
	Session(Muika &m, int64_t chat_id, const std::string &deck_name);
	~Session(void);
	int start(void);
	inline int64_t getChatId(void) const { return chat_id_; }
	void drawAndSendCard(void);
	void handleAnswer(TgBot::Message::Ptr &msg);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__SESSION_HPP */
