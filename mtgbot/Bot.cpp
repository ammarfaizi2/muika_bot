// SPDX-License-Identifier: GPL-2.0-only
#include <mtgbot/Bot.hpp>
#include <mtgbot/Reactor.hpp>

#ifndef HAVE_CURL
#define HAVE_CURL
#endif

#include <tgbot/tgbot.h>
#include <tgbot/net/CurlHttpClient.h>

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <pthread.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

namespace mtgbot {

Bot::Bot(const BotConfig &cfg):
	cfg_(cfg)
{
	muika::MuikaConfig muika_cfg;
	reactor_ = std::make_shared<Reactor>(this);
	muika_cfg.storage_path = cfg.storage_path + "/muika";
	muika_ = std::make_unique<muika::Muika>(muika_cfg, reactor_);
	muika_->loadModule("hello");
	http_client_ = std::make_unique<TgBot::CurlHttpClient>();
	bot_ = std::make_unique<TgBot::Bot>(cfg_.token, *http_client_);
	mgr_ = std::make_unique<BotMgr>(this);
}

TgBot::Bot *Bot::bot(void)
{
	return bot_.get();
}

Bot::~Bot(void)
{
}

class BotMgr {
public:
	inline BotMgr(Bot *bot);
	inline ~BotMgr(void);
	inline void run(void);
	inline void stop(void);

private:
	inline void runWorker(unsigned int id);
	inline void handleMsgText(const TgBot::Message::Ptr &msg);
	inline void enqueueMsg(TgBot::Message::Ptr msg);
	inline TgBot::Message::Ptr dequeueMsg(void);

	Bot *bot_;
	std::thread workers_[16];
	std::queue<TgBot::Message::Ptr> msg_queue_;
	uint32_t nr_sleeping_ = 0;
	std::mutex msg_queue_lock_;
	std::condition_variable msg_queue_cv_;
};

inline BotMgr::BotMgr(Bot *bot):
	bot_(bot)
{
}

inline BotMgr::~BotMgr(void)
{
}

inline void BotMgr::run(void)
{
	for (unsigned int i = 0; i < ARRAY_SIZE(workers_); i++)
		workers_[i] = std::thread(&BotMgr::runWorker, this, i);

	bot_->bot()->getEvents().onAnyMessage([this](TgBot::Message::Ptr msg) {
		printf("Received message: %s\n", msg->text.c_str());
		if (!msg->text.empty())
			enqueueMsg(msg);
	});

	TgBot::TgLongPoll longPoll(*bot_->bot());
	while (true) {
		try {
			while (true)
				longPoll.start();
		} catch (std::exception &e) {
			printf("Error: %s\n", e.what());
		} catch (...) {
			printf("Unknown error\n");
		}

		sleep(3);
	}
}

inline void BotMgr::stop(void)
{
	{
		std::unique_lock<std::mutex> lk(msg_queue_lock_);
		msg_queue_cv_.notify_all();
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(workers_); i++)
		workers_[i].join();
}

inline void BotMgr::enqueueMsg(const TgBot::Message::Ptr msg)
{
	std::unique_lock<std::mutex> lk(msg_queue_lock_);
	msg_queue_.push(msg);
	if (nr_sleeping_ > 0)
		msg_queue_cv_.notify_one();
}

inline TgBot::Message::Ptr BotMgr::dequeueMsg(void)
{
	std::unique_lock<std::mutex> lk(msg_queue_lock_);
	nr_sleeping_++;
	msg_queue_cv_.wait(lk, [this] { return !msg_queue_.empty(); });
	nr_sleeping_--;
	auto msg = msg_queue_.front();
	msg_queue_.pop();
	return msg;
}

inline void BotMgr::handleMsgText(const TgBot::Message::Ptr &msg)
{
	std::string name = msg->from->firstName;
	if (!msg->from->lastName.empty())
		name += " " + msg->from->lastName;

	auto mmsg = muika::Muika::createMsgText(
		std::to_string(msg->chat->id),
		std::to_string(msg->messageId),
		std::to_string(msg->from->id),
		name,
		msg->from->username,
		msg->text
	);
	bot_->muika_->passMsg(mmsg);
}

inline void BotMgr::runWorker(unsigned int id)
{
	char tname[128];

	snprintf(tname, sizeof(tname), "mtgwrk-%u", id);
	pthread_setname_np(pthread_self(), tname);

	while (!bot_->should_stop_) {
		TgBot::Message::Ptr msg = dequeueMsg();

		if (!msg ->text.empty()) {
			printf("Worker %u: handling text message %s\n", id, msg->text.c_str());
			handleMsgText(msg);
			continue;
		}
	}
}

void Bot::run(void)
{
	printf("Bot is running...\n");
	printf("Bot username: @%s\n", bot_->getApi().getMe()->username.c_str());
	mgr_->run();
}

void Bot::stop(void)
{
	should_stop_ = true;
	mgr_->stop();
}

} /* namespace mtgbot */
