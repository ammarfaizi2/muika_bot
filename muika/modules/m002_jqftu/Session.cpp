// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Session.hpp>
#include <muika/modules/m002_jqftu/WorkerPool.hpp>
#include <muika/modules/m002_jqftu/internal.hpp>
#include <muika/Muika.hpp>
#include <muika/helpers.hpp>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

#include <dirent.h>

namespace muika {
namespace modules {
namespace m002_jqftu {

std::string Session::sessionFilePath(void) const
{
	char buf[64];
	snprintf(buf, sizeof(buf), "/s_%lld.json", (long long)chat_id_);
	return paths_.sessions_dir + buf;
}

Session::Session(muika::Module *mod, WorkerPool *pool, const Paths &paths,
		 int64_t chat_id, uint64_t last_msg_id):
	mod_(mod),
	pool_(pool),
	paths_(paths),
	chat_id_(chat_id),
	last_msg_id_(last_msg_id),
	deck_group_(paths.decks_dir)
{
}

uint64_t Session::sendMsg(const std::string &msg, uint64_t reply_to, bool save_last)
{
	OutboundMessage om;
	om.text = msg;
	om.parse_mode = "HTML";
	if (reply_to)
		om.reply_to_msg_id = std::to_string(reply_to);

	char chat_buf[32];
	snprintf(chat_buf, sizeof(chat_buf), "%lld", (long long)chat_id_);

	std::string id = mod_->reactor()->sendMsgText(chat_buf, om);
	uint64_t mid = id.empty() ? 0 : strtoull(id.c_str(), nullptr, 10);
	if (save_last && mid)
		last_msg_id_ = mid;
	return mid;
}

uint64_t Session::sendPhoto(const std::string &photo_url,
			    const std::string &caption,
			    uint64_t reply_to)
{
	OutboundMessage om;
	om.text = caption;
	if (reply_to)
		om.reply_to_msg_id = std::to_string(reply_to);

	char chat_buf[32];
	snprintf(chat_buf, sizeof(chat_buf), "%lld", (long long)chat_id_);

	std::string id = mod_->reactor()->sendMsgPhotoUrl(chat_buf, photo_url, om);
	uint64_t mid = id.empty() ? 0 : strtoull(id.c_str(), nullptr, 10);
	if (mid)
		last_msg_id_ = mid;
	return mid;
}

inline std::string Session::__generateScoreBoard(void)
{
	return generateScoreBoard(points_);
}

std::string Session::generateScoreBoard(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	return __generateScoreBoard();
}

inline void Session::__resetCurrent(void)
{
	current_deck_ = nullptr;
	current_card_ = nullptr;
}

inline void Session::__setCurrent(Deck *deck, Card *card)
{
	current_deck_ = deck;
	current_card_ = card;
}

bool Session::__sendCard(Deck *deck, Card *card)
{
	try {
		assert(!current_deck_);
		assert(!current_card_);

		std::string caption;
		caption += "Scope: " + deck->getScope() + "\n\n";
		caption += card->getCardCaption() + "\n\n";
		caption += "Timeout: " + std::to_string(timeout_secs_) + " seconds\n";
		caption += "Romaji fault tolerance: " +
			   std::to_string(card->getRomajiFaultTolerance()) + "%\n";

		std::string img_url = card->getCardImage();
		sendPhoto(img_url, caption, last_msg_id_);
		return true;
	} catch (const std::exception &e) {
		pr_debug("Send card fail, chat_id=%lld, deck=%s, card=%s: %s",
			 (long long)chat_id_, deck->getName().c_str(),
			 card->getCardAnswer().c_str(), e.what());
		return false;
	}
}

bool Session::__sendCardMayRetry(Deck *deck, Card *card,
				 std::unique_lock<std::mutex> &lock,
				 uint32_t try_num)
{
	uint32_t total = try_num;

	for (uint32_t i = 0; i < total; i++) {
		if (__sendCard(deck, card))
			return true;

		sendMsg("Failed to send card, retrying in 5 seconds...", last_msg_id_);
		cond_.wait_for(lock, std::chrono::seconds(5));
		if (should_stop_)
			return false;
	}

	sendMsg("Failed to send card after " + std::to_string(total) +
		" retries, giving up...", last_msg_id_);
	return false;
}

void Session::__sendFailMessage(const std::string &reason)
{
	assert(current_deck_);
	assert(current_card_);
	sendMsg(reason + "\n\n" + current_card_->getCardDetails(), last_msg_id_);
}

inline void Session::__sendStartMessage(void)
{
	sendMsg("Session started!", last_msg_id_);
	sendMsg("Shuffling decks...", last_msg_id_);
}

inline void Session::__sendEndMessage(void)
{
	sendMsg("Session ended!\n\n" + __generateScoreBoard(), last_msg_id_);
}

void Session::__workerFunc(std::unique_lock<std::mutex> &lock)
{
	bool first_card = true;

	if (!silent_start_) {
		__sendStartMessage();
		deck_group_.shuffleAllDecks();
		cond_.wait_for(lock, std::chrono::seconds(5));
	}

	__saveToDisk();
	while (!should_stop_) {
		Deck *deck = deck_group_.drawDeck();
		if (!deck)
			break;

		Card *card = deck->drawCard();
		assert(card);

		__saveToDisk();

		if (first_card) {
			first_card = false;
		} else {
			cond_.wait_for(lock, std::chrono::seconds(next_card_delay_secs_));
			if (should_stop_)
				break;
		}

		if (!__sendCardMayRetry(deck, card, lock, 5))
			break;

		__setCurrent(deck, card);
		cond_.wait_for(lock, std::chrono::seconds(timeout_secs_));

		if (current_card_) {
			std::string msg = should_stop_ ? "Game is stopped." : "Time's up!";
			__sendFailMessage(msg);
			__resetCurrent();
		}

		if (should_stop_)
			break;
	}

	__deleteFromDisk();
	__sendEndMessage();
}

void Session::workerFunc(void)
{
	std::unique_lock<std::mutex> lock(mutex_);

	try {
		__workerFunc(lock);
	} catch (const std::exception &e) {
		pr_debug("Session worker thread failed: %s", e.what());
		try {
			sendMsg("Session worker thread error: " +
				std::string(e.what()), last_msg_id_);
		} catch (...) {
		}
	}

	should_stop_ = true;
	worker_running_ = false;
}

void Session::start(bool silent)
{
	std::shared_ptr<Session> self;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (worker_running_)
			throw std::runtime_error("Session worker already running");

		try {
			loadAllPointsFromDisk(paths_.points_dir, chat_id_, points_);
		} catch (...) {
		}
		silent_start_ = silent;
		should_stop_ = false;
		worker_running_ = true;
		self = shared_from_this();
	}

	pool_->submit([self]() {
		self->workerFunc();
	});
}

void Session::stop(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	should_stop_ = true;
	cond_.notify_one();
}

Point Session::__tryLoadPointFromDisk(uint64_t user_id, uint64_t err_reply_to)
{
	try {
		return Point::tryLoadFromDisk(paths_.points_dir, chat_id_, user_id);
	} catch (const std::exception &e) {
		pr_debug("Failed to load point from disk, chat_id=%lld, user_id=%llu: %s",
			 (long long)chat_id_, (unsigned long long)user_id, e.what());

		std::string msg = "Failed to load point from disk: " +
				  std::string(e.what()) +
				  "\n\nYou will be treated as a new player.";
		sendMsg(msg, err_reply_to, false);
		return Point(user_id, 0, "", "", "");
	}
}

uint64_t Session::__handleCorrectAnswerPoint(uint64_t user_id,
					     const std::string &username,
					     const std::string &first_name,
					     const std::string &last_name,
					     uint64_t reply_to)
{
	Point *p;
	auto i = points_.find(user_id);
	if (i == points_.end()) {
		Point tmp = __tryLoadPointFromDisk(user_id, reply_to);
		if (tmp.getUserId() == 0)
			tmp.setUserId(user_id);
		tmp.setUsername(username);
		tmp.setFirstName(first_name);
		tmp.setLastName(last_name);
		points_.emplace(user_id, tmp);
		p = &points_.at(user_id);
	} else {
		p = &i->second;
		p->setUsername(username);
		p->setFirstName(first_name);
		p->setLastName(last_name);
	}

	p->addPoint(1);

	try {
		p->saveToDisk(paths_.points_dir, chat_id_);
	} catch (const std::exception &e) {
		pr_debug("Failed to save point to disk, chat_id=%lld, user_id=%llu: %s",
			 (long long)chat_id_, (unsigned long long)user_id, e.what());
		sendMsg("Failed to save point to disk: " +
			std::string(e.what()), reply_to, false);
	}

	return p->getPoint();
}

bool Session::answer(MsgPtr msg)
{
	std::unique_lock<std::mutex> lock(mutex_);

	if (!current_card_)
		return false;

	const std::string &chat = msg->chat_id();
	int64_t chat_id = strtoll(chat.c_str(), nullptr, 10);
	if (chat_id != chat_id_) {
		pr_debug("Session::answer: chat_id mismatch, got=%s expected=%lld",
			 chat.c_str(), (long long)chat_id_);
		return false;
	}

	const std::string &txt = msg->content().text();
	if (txt.empty())
		return false;

	if (!current_card_->checkAnswer(txt))
		return false;

	uint64_t user_id = strtoull(msg->user().id().c_str(), nullptr, 10);
	uint64_t reply_to = strtoull(msg->id().c_str(), nullptr, 10);

	uint64_t point = __handleCorrectAnswerPoint(user_id,
						    msg->user().uname(),
						    msg->user().name(),
						    "",
						    reply_to);

	std::string reply = "Correct, +1 point!\n\n"
			    "Your point is: " + std::to_string(point) + "\n\n" +
			    current_card_->getCardDetails();
	sendMsg(reply, reply_to, false);
	__resetCurrent();
	cond_.notify_one();
	return true;
}

void Session::setTimeout(uint32_t secs, bool skip_current)
{
	std::lock_guard<std::mutex> lock(mutex_);
	timeout_secs_ = secs;
	if (skip_current)
		cond_.notify_one();
}

void Session::setNextCardDelay(uint32_t secs, bool skip_current)
{
	std::lock_guard<std::mutex> lock(mutex_);
	next_card_delay_secs_ = secs;
	if (skip_current)
		cond_.notify_one();
}

Point Session::getPoint(uint64_t user_id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto i = points_.find(user_id);
	if (i == points_.end())
		return Point(user_id, 0, "", "", "");
	return i->second;
}

inline std::string Session::__toJsonString(void)
{
	::nlohmann::json j;

	j["chat_id"] = chat_id_;
	j["last_msg_id"] = last_msg_id_;
	j["timeout_secs"] = timeout_secs_;
	j["next_card_delay_secs"] = next_card_delay_secs_;
	j["deck_group"] = deck_group_.toJson();
	return j.dump(1, '\t');
}

std::string Session::toJsonString(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	return __toJsonString();
}

inline void Session::__deleteFromDisk(void)
{
	std::string p = sessionFilePath();
	remove(p.c_str());
}

void Session::__createSessionDir(void)
{
	if (mk_mkdir_p(paths_.sessions_dir.c_str(), 0755) < 0)
		throw std::runtime_error("Failed to create sessions dir: " +
					 paths_.sessions_dir);
}

inline void Session::__saveToDisk(void)
{
	__createSessionDir();
	std::string path = sessionFilePath();
	int r = mk_file_put_contents(path, __toJsonString());
	if (r < 0)
		throw std::runtime_error("Failed to write session file: " + path);
}

inline void Session::__addDeckByName(const std::string &name)
{
	deck_group_.addDeckByName(name);
}

void Session::saveToDisk(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	__saveToDisk();
}

void Session::addDeckByName(const std::string &name)
{
	std::lock_guard<std::mutex> lock(mutex_);
	__addDeckByName(name);
}

void Session::sendRebootMessage(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	sendMsg("The system was rebooted, trying to recover the session...",
		last_msg_id_);
}

// static
std::shared_ptr<Session> Session::fromJsonString(muika::Module *mod,
						 WorkerPool *pool,
						 const Paths &paths,
						 const std::string &json_str)
{
	::nlohmann::json j = ::nlohmann::json::parse(json_str);

	if (!j.is_object())
		throw std::runtime_error("Session JSON is not an object");

	if (!j.contains("chat_id") || !j["chat_id"].is_number_integer())
		throw std::runtime_error("Missing/invalid chat_id");
	if (!j.contains("last_msg_id") || !j["last_msg_id"].is_number_unsigned())
		throw std::runtime_error("Missing/invalid last_msg_id");
	if (!j.contains("timeout_secs") || !j["timeout_secs"].is_number_unsigned())
		throw std::runtime_error("Missing/invalid timeout_secs");
	if (!j.contains("next_card_delay_secs") ||
	    !j["next_card_delay_secs"].is_number_unsigned())
		throw std::runtime_error("Missing/invalid next_card_delay_secs");
	if (!j.contains("deck_group") || !j["deck_group"].is_object())
		throw std::runtime_error("Missing/invalid deck_group");

	auto r = std::make_shared<Session>(mod, pool, paths,
					   j["chat_id"].get<int64_t>(),
					   j["last_msg_id"].get<uint64_t>());
	r->setTimeout(j["timeout_secs"].get<uint32_t>());
	r->setNextCardDelay(j["next_card_delay_secs"].get<uint32_t>());
	r->deck_group_.fromJson(j["deck_group"]);
	return r;
}

// static
void Session::loadAllPointsFromDisk(const std::string &points_dir,
				    int64_t chat_id,
				    std::unordered_map<uint64_t, Point> &points)
{
	std::string dir = Point::chatPointsDir(points_dir, chat_id);
	DIR *d = opendir(dir.c_str());
	if (!d)
		return;

	while (true) {
		struct dirent *ent = readdir(d);
		if (!ent)
			break;

		if (ent->d_type != DT_REG && ent->d_type != DT_UNKNOWN)
			continue;

		size_t len = strlen(ent->d_name);
		if (len < 5 || strcmp(ent->d_name + len - 5, ".json"))
			continue;

		uint64_t user_id = strtoull(ent->d_name, nullptr, 10);
		if (!user_id)
			continue;

		try {
			Point p = Point::tryLoadFromDisk(points_dir, chat_id, user_id);
			if (p.getUserId() == user_id)
				points.emplace(user_id, p);
		} catch (const std::exception &e) {
			pr_debug("Failed to load point: chat=%lld user=%llu: %s",
				 (long long)chat_id,
				 (unsigned long long)user_id, e.what());
		}
	}

	closedir(d);
}

// static
std::string Session::generateScoreBoard(std::unordered_map<uint64_t, Point> &points)
{
	std::vector<std::pair<uint64_t, Point>> v(points.begin(), points.end());
	std::sort(v.begin(), v.end(), [](const auto &a, const auto &b) {
		return a.second.getPoint() > b.second.getPoint();
	});

	std::string ret;
	uint32_t i = 1;
	for (auto &s : v) {
		const Point &p = s.second;
		std::string name = mk_htmlspecialchars(p.getFullName());
		name = "<a href=\"tg://user?id=" + std::to_string(p.getUserId()) +
		       "\">" + name + "</a>";

		ret += std::to_string(i++) + ". " + name + ": " +
		       std::to_string(p.getPoint()) + " point";
		if (p.getPoint() > 1)
			ret += "s";
		ret += "\n";
	}
	return ret;
}

// static
std::string Session::generateScoreBoardFromDisk(const std::string &points_dir,
						int64_t chat_id)
{
	std::unordered_map<uint64_t, Point> points;
	loadAllPointsFromDisk(points_dir, chat_id, points);
	return generateScoreBoard(points);
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
