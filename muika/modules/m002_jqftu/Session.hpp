// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__SESSION_HPP
#define MUIKA__MODULES__M002_JQFTU__SESSION_HPP

#include <muika/Module.hpp>
#include <muika/modules/m002_jqftu/DeckGroup.hpp>
#include <muika/modules/m002_jqftu/Point.hpp>
#include <muika/modules/m002_jqftu/internal.hpp>

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace muika {
namespace modules {
namespace m002_jqftu {

class WorkerPool;

class Session: public std::enable_shared_from_this<Session> {
private:
	muika::Module *mod_;
	WorkerPool *pool_;
	Paths paths_;

	std::mutex mutex_;
	std::condition_variable cond_;
	bool should_stop_ = false;
	bool silent_start_ = false;
	bool worker_running_ = false;

	int64_t chat_id_;
	uint64_t last_msg_id_ = 0;
	uint32_t timeout_secs_ = 3600;
	uint32_t next_card_delay_secs_ = 5;
	DeckGroup deck_group_;

	std::unordered_map<uint64_t, Point> points_;
	Deck *current_deck_ = nullptr;
	Card *current_card_ = nullptr;

	uint64_t sendMsg(const std::string &msg, uint64_t reply_to = 0,
			 bool save_last = true);
	uint64_t sendPhoto(const std::string &photo_url,
			   const std::string &caption,
			   uint64_t reply_to);
	void workerFunc(void);
	void __workerFunc(std::unique_lock<std::mutex> &lock);
	void __sendFailMessage(const std::string &reason);
	void __resetCurrent(void);
	void __setCurrent(Deck *deck, Card *card);
	bool __sendCard(Deck *deck, Card *card);
	bool __sendCardMayRetry(Deck *deck, Card *card,
				std::unique_lock<std::mutex> &lock,
				uint32_t try_num);
	void __sendStartMessage(void);
	void __sendEndMessage(void);
	std::string __generateScoreBoard(void);
	std::string __toJsonString(void);
	uint64_t __handleCorrectAnswerPoint(uint64_t user_id,
					    const std::string &username,
					    const std::string &first_name,
					    const std::string &last_name,
					    uint64_t reply_to);
	Point __tryLoadPointFromDisk(uint64_t user_id, uint64_t err_reply_to);
	void __addDeckByName(const std::string &name);
	void __saveToDisk(void);
	void __deleteFromDisk(void);
	void __createSessionDir(void);

	std::string sessionFilePath(void) const;

public:
	Session(muika::Module *mod, WorkerPool *pool, const Paths &paths,
		int64_t chat_id, uint64_t last_msg_id = 0);
	~Session(void) = default;

	void start(bool silent = false);
	void stop(void);
	void setTimeout(uint32_t secs, bool skip_current = false);
	void setNextCardDelay(uint32_t secs, bool skip_current = false);
	bool answer(MsgPtr msg);
	std::string generateScoreBoard(void);
	Point getPoint(uint64_t user_id);
	void addDeckByName(const std::string &name);
	void saveToDisk(void);
	void sendRebootMessage(void);

	inline bool getShouldStop(void) const { return should_stop_; }
	int64_t getChatId(void) const { return chat_id_; }

	std::string toJsonString(void);
	static std::shared_ptr<Session> fromJsonString(muika::Module *mod,
						       WorkerPool *pool,
						       const Paths &paths,
						       const std::string &json_str);
	static std::string generateScoreBoard(std::unordered_map<uint64_t, Point> &points);
	static std::string generateScoreBoardFromDisk(const std::string &points_dir,
						      int64_t chat_id);
	static void loadAllPointsFromDisk(const std::string &points_dir,
					  int64_t chat_id,
					  std::unordered_map<uint64_t, Point> &points);
};

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__M002_JQFTU__SESSION_HPP */
