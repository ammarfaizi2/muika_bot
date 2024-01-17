// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Session.hpp"

#include <unordered_map>
#include <stdexcept>

namespace muika {
namespace modules {
namespace jqftu {

static std::unordered_map<int64_t, Session *> g_sessions;
static std::mutex g_sessions_mutex;

Session::Session(Muika &m, int64_t chat_id, const std::string &deck_name):
	m_(m),
	chat_id_(chat_id)
{
	ref_count_ = 0;
	deck_ = Deck::createDeck(deck_name);
}

Session::~Session(void)
{
}

inline void Session::worker(void)
{
	ref_count_++;
	std::unique_lock<std::mutex> lock(mutex_);

	while (!should_stop_ && !deck_->isFinished()) {
		cond_.wait_for(lock, std::chrono::seconds(timeout_));
	}

	lock.unlock();
	deleteSession(chat_id_);
	if (ref_count_-- == 1)
		delete this;
}

void Session::start(void)
{
	try {
		worker_ = std::thread(&Session::worker, this);
	} catch (const std::exception &e) {
		m_.getBot().getApi().sendMessage(
			chat_id_,
			"Failed to start session: " + std::string(e.what())
		);
		return;
	}

	worker_.detach();
	m_.getBot().getApi().sendMessage(chat_id_, "Session started!");
}

void Session::stop(void)
{
	std::unique_lock<std::mutex> lock(mutex_);
	should_stop_ = true;
	cond_.notify_one();
}

void Session::answer(const TgBot::Message::Ptr &msg)
{
	std::unique_lock<std::mutex> lock(mutex_);
	std::string reply, answer = msg->text;
	Score *s = nullptr;

	if (!current_card_ || !current_card_->answer(answer))
		return;

	auto it = scores_.find(msg->from->id);
	if (it == scores_.end()) {
		scores_.emplace(msg->from->id, Score{1});
		s = &scores_.at(msg->from->id);
	} else {
		s = &it->second;
		s->point_++;
	}

	reply =	"Correct!\n"
		"Your point is: " + std::to_string(s->point_) + "\n\n" +
		current_card_->getInfo();

	m_.getBot().getApi().sendMessage(chat_id_, reply, false, msg->messageId);
	current_card_ = nullptr;
	cond_.notify_one();
}

// static
Session *Session::getSession(int64_t chat_id)
{
	std::unique_lock<std::mutex> lock(g_sessions_mutex);
	Session *sess;

	auto it = g_sessions.find(chat_id);
	if (it == g_sessions.end())
		return nullptr;

	sess = it->second;
	sess->ref_count_++;
	return sess;
}

// static
void Session::putSession(Session *sess)
{
	if (sess->ref_count_-- == 1)
		delete sess;
}

// static
Session *Session::createSession(Muika &m, int64_t chat_id, const std::string &deck_name)
{
	std::unique_lock<std::mutex> lock(g_sessions_mutex);
	Session *sess = nullptr;

	auto it = g_sessions.find(chat_id);
	if (it != g_sessions.end())
		return nullptr;

	try {
		sess = new Session(m, chat_id, deck_name);
		g_sessions.emplace(chat_id, sess);
		sess->ref_count_++;
	} catch (const std::exception &e) {
		delete sess;
		sess = nullptr;
	}
	return sess;
}

// static
void Session::deleteSession(int64_t chat_id)
{
	std::unique_lock<std::mutex> lock(g_sessions_mutex);

	auto it = g_sessions.find(chat_id);
	if (it == g_sessions.end())
		return;

	g_sessions.erase(it);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
