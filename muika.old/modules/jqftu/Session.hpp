// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__SESSION_HPP
#define MUIKA__MODULES__JQFTU__SESSION_HPP

#include "muika/Muika.hpp"
#include "muika/helpers.hpp"
#include "muika/modules/jqftu/Point.hpp"
#include "muika/modules/jqftu/DeckGroup.hpp"

#include <condition_variable>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>

namespace muika {
namespace modules {
namespace jqftu {

class Session {
private:
	Muika &m_;

	std::mutex mutex_;
	volatile bool should_stop_ = false;
	bool silent_start_ = false;

	__serialize int64_t chat_id_;
	__serialize uint64_t last_msg_id_ = 0;
	__serialize uint32_t timeout_secs_ = 3600;
	__serialize uint32_t next_card_delay_secs_ = 5;
	__serialize DeckGroup deck_group_;

	std::unordered_map<uint64_t, Point> points_;
	Deck *current_deck_ = nullptr;
	Card *current_card_ = nullptr;

	std::thread thread_;
	std::condition_variable cond_;

	std::shared_ptr<Session> *self_ptr_ = nullptr;

	uint64_t sendMsg(const std::string &msg, uint64_t reply_to = 0, bool save_last = true);
	void workerFunc(void);
	__must_hold(&mutex_) void __sendFailMessage(const std::string &reason);
	__must_hold(&mutex_) void __workerFunc(std::unique_lock<std::mutex> &lock);
	__must_hold(&mutex_) void __resetCurrent(void);
	__must_hold(&mutex_) void __setCurrent(Deck *deck, Card *card);
	__must_hold(&mutex_) bool __sendCard(Deck *deck, Card *card);
	__must_hold(&mutex_) bool __sendCardMayRetry(Deck *deck, Card *card, std::unique_lock<std::mutex> &lock, uint32_t try_num);
	__must_hold(&mutex_) void __sendStartMessage(void);
	__must_hold(&mutex_) void __sendEndMessage(void);
	__must_hold(&mutex_) void __sendScoreBoard(void);
	__must_hold(&mutex_) std::string __generateScoreBoard(void);
	__must_hold(&mutex_) std::string __toJsonString(void);
	__must_hold(&mutex_) uint64_t __handleCorrectAnswerPoint(const TgBot::Message::Ptr &msg);
	__must_hold(&mutex_) Point __tryLoadPointFromDisk(uint64_t user_id, uint64_t err_reply_to);
	__must_hold(&mutex_) std::shared_ptr<Session> __waitForSelfPtr(std::unique_lock<std::mutex> &lock);
	__must_hold(&mutex_) void __addDeckByName(const std::string &name);
	__must_hold(&mutex_) void __saveToDisk(void);
	__must_hold(&mutex_) void __deleteFromDisk(void);
	__must_hold(&mutex_) void __createSessionDir(void);

public:
	Session(Muika &m, int64_t chat_id = 0, uint64_t last_msg_id = 0);
	~Session(void) = default;

	void start(bool silent = false);
	void stop(void);
	void setTimeout(uint32_t secs, bool skip_current = false);
	void setNextCardDelay(uint32_t secs, bool skip_current = false);
	bool answer(const TgBot::Message::Ptr &msg);
	std::string generateScoreBoard(void);
	void giveSelfPtr(std::shared_ptr<Session> *self_ptr);
	Point getPoint(uint64_t user_id);
	void addDeckByName(const std::string &name);
	void saveToDisk(void);
	void sendRebootMessage(void);

	inline bool getShouldStop(void) const { return should_stop_; }
	int64_t getChatId(void) const { return chat_id_; }

	std::string toJsonString(void);
	static std::shared_ptr<Session> fromJsonString(Muika &m, const std::string &json_str);
	static std::string generateScoreBoard(std::unordered_map<uint64_t, Point> &points);
	static std::string generateScoreBoardFromDisk(int64_t chat_id);
	static void loadAllPointsFromDisk(int64_t chat_id, std::unordered_map<uint64_t, Point> &points);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__SESSION_HPP */
