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

static void loadModules(muika::Muika *m)
{
	std::vector<std::string> modules = {
		"hello",
		"jqftu"
	};

	for (const auto &mod: modules) {
		try {
			m->loadModule(mod);
		} catch (std::exception &e) {
			printf("Error loading module %s: %s\n", mod.c_str(), e.what());
		} catch (...) {
			printf("Unknown error loading module %s\n", mod.c_str());
		}
	}
}

Bot::Bot(const BotConfig &cfg):
	cfg_(cfg)
{
	muika::MuikaConfig muika_cfg;
	reactor_ = std::make_shared<Reactor>(this);
	muika_cfg.storage_path = cfg.storage_path + "/muika";
	muika_ = std::make_unique<muika::Muika>(muika_cfg, reactor_);
	http_client_ = std::make_unique<TgBot::CurlHttpClient>();
	bot_ = std::make_unique<TgBot::Bot>(cfg_.token, *http_client_);
	mgr_ = std::make_unique<BotMgr>(this);
	loadModules(muika_.get());
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
	struct InboundEvent {
		TgBot::Message::Ptr msg;
		TgBot::CallbackQuery::Ptr cb;
	};

	inline void runWorker(unsigned int id);
	inline void handleEvent(const InboundEvent &ev);
	inline void handleMsgText(const TgBot::Message::Ptr &msg);
	inline void handleCallback(const TgBot::CallbackQuery::Ptr &cb);
	inline void enqueueEvent(InboundEvent ev);
	inline InboundEvent dequeueEvent(void);

	Bot *bot_;
	std::thread workers_[16];
	std::queue<InboundEvent> msg_queue_;
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
			enqueueEvent({msg, nullptr});
	});

	bot_->bot()->getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr cb) {
		printf("Received callback query: %s\n", cb->data.c_str());
		enqueueEvent({nullptr, cb});
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

inline void BotMgr::enqueueEvent(InboundEvent ev)
{
	std::unique_lock<std::mutex> lk(msg_queue_lock_);
	msg_queue_.push(std::move(ev));
	if (nr_sleeping_ > 0)
		msg_queue_cv_.notify_one();
}

inline BotMgr::InboundEvent BotMgr::dequeueEvent(void)
{
	std::unique_lock<std::mutex> lk(msg_queue_lock_);
	nr_sleeping_++;
	msg_queue_cv_.wait(lk, [this] { return !msg_queue_.empty(); });
	nr_sleeping_--;
	auto ev = std::move(msg_queue_.front());
	msg_queue_.pop();
	return ev;
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

inline void BotMgr::handleCallback(const TgBot::CallbackQuery::Ptr &cb)
{
	if (!cb->from)
		return;

	std::string name = cb->from->firstName;
	if (!cb->from->lastName.empty())
		name += " " + cb->from->lastName;

	std::string chat_id;
	std::string msg_id;
	if (cb->message) {
		if (cb->message->chat)
			chat_id = std::to_string(cb->message->chat->id);
		msg_id = std::to_string(cb->message->messageId);
	}

	auto mmsg = muika::Muika::createMsgCallback(
		chat_id,
		msg_id,
		cb->id,
		std::to_string(cb->from->id),
		name,
		cb->from->username,
		cb->data
	);
	bot_->muika_->passMsg(mmsg);

	/* Auto-acknowledge the callback so Telegram stops the spinner. */
	try {
		bot_->bot()->getApi().answerCallbackQuery(cb->id);
	} catch (const std::exception &e) {
		printf("answerCallbackQuery failed: %s\n", e.what());
	}
}

inline void BotMgr::handleEvent(const InboundEvent &ev)
{
	if (ev.msg)
		handleMsgText(ev.msg);
	else if (ev.cb)
		handleCallback(ev.cb);
}

inline void BotMgr::runWorker(unsigned int id)
{
	char tname[128];

	snprintf(tname, sizeof(tname), "mtgwrk-%u", id);
	pthread_setname_np(pthread_self(), tname);

	while (!bot_->should_stop_) {
		InboundEvent ev = dequeueEvent();
		handleEvent(ev);
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
