// SPDX-License-Identifier: GPL-2.0-only
#ifndef MTGBOT__BOT_HPP
#define MTGBOT__BOT_HPP

#include <muika/Muika.hpp>
#include <mlogger/mlogger.h>

#include <string>
#include <vector>
#include <memory>

namespace TgBot {
class Bot;
class HttpClient;
class Message;
} /* namespace TgBot */

namespace mtgbot {

class Reactor;
class BotMgr;

struct BotConfig {
	std::string token;
	std::string storage_path;
	std::vector<int64_t> super_users;
	/*
	 * Optional logger; not owned by Bot. Caller must keep it alive
	 * for the lifetime of Bot and free it afterwards.
	 */
	mk_logger_t *logger = nullptr;
};

class Bot {
public:
	Bot(const BotConfig &cfg);
	~Bot(void);
	void run(void);
	TgBot::Bot *bot(void);
	void stop(void);
	mk_logger_t *logger(void) const { return cfg_.logger; }

private:
	void runWorker(unsigned int id);
	volatile bool should_stop_ = false;
	std::unique_ptr<muika::Muika> muika_;
	std::shared_ptr<Reactor> reactor_;
	std::unique_ptr<TgBot::HttpClient> http_client_;
	std::unique_ptr<TgBot::Bot> bot_;
	std::unique_ptr<BotMgr> mgr_;
	BotConfig cfg_;

	friend class BotMgr;
};

} /* namespace mtgbot */

#endif /* MTGBOT__BOT_HPP */
