// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Session.hpp"

#include <cstdio>
#include <cerrno>

#include <dirent.h>

using muika::htmlspecialchars;

namespace muika {
namespace modules {
namespace jqftu {

Session::Session(Muika &m, int64_t chat_id, uint64_t last_msg_id):
	m_(m),
	chat_id_(chat_id),
	last_msg_id_(last_msg_id)
{
}

inline uint64_t Session::sendMsg(const std::string &msg, uint64_t reply_to, bool save_last)
{
	auto i = m_.getApi().sendMessage(chat_id_, msg, false, reply_to, nullptr, "HTML");
	if (save_last)
		last_msg_id_ = i->messageId;

	return i->messageId;
}

inline std::string Session::__generateScoreBoard(void)
	__must_hold(&mutex_)
{
	return generateScoreBoard(points_);
}

std::string Session::generateScoreBoard(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	return __generateScoreBoard();
}

inline void Session::__resetCurrent(void)
	__must_hold(&mutex_)
{
	current_deck_ = nullptr;
	current_card_ = nullptr;
}

inline void Session::__setCurrent(Deck *deck, Card *card)
	__must_hold(&mutex_)
{
	current_deck_ = deck;
	current_card_ = card;
}

inline bool Session::__sendCard(Deck *deck, Card *card)
	__must_hold(&mutex_)
{
	try {
		std::string img_url, caption = "";

		assert(!current_deck_);
		assert(!current_card_);

		caption += "Scope: " + deck->getScope() + "\n\n";
		caption += card->getCardCaption() + "\n\n";
		caption += "Timeout: " + std::to_string(timeout_secs_) + " seconds\n";
		caption += "Romaji fault tolerance: " + std::to_string(card->getRomajiFaultTolerance()) + "%\n";
		img_url = card->getCardImage();

		auto i = m_.getApi().sendPhoto(chat_id_, img_url, caption, last_msg_id_);
		last_msg_id_ = i->messageId;
		return true;
	} catch (const std::exception &e) {
		pr_debug("Send card fail, chat_id=%lld, deck=%s, card=%s: %s",
			 (long long)chat_id_, deck->getName().c_str(),
			 card->getCardAnswer().c_str(), e.what());
		return false;
	}
}

inline
bool Session::__sendCardMayRetry(Deck *deck, Card *card, std::unique_lock<std::mutex> &lock, uint32_t try_num)
	__must_hold(&mutex_)
{
	while (try_num--) {
		if (__sendCard(deck, card))
			return true;

		sendMsg("Failed to send card, retrying in 5 seconds...", last_msg_id_);
		cond_.wait_for(lock, std::chrono::seconds(5));
	}

	/*
	 * Give up...
	 */
	sendMsg("Failed to send card after " + std::to_string(try_num) + " retries, giving up...", last_msg_id_);
	return false;
}

void Session::__sendFailMessage(const std::string &reason)
	__must_hold(&mutex_)
{
	assert(current_deck_);
	assert(current_card_);
	sendMsg(reason + "\n\n" + current_card_->getCardDetails(), last_msg_id_);
}

inline void Session::__sendStartMessage(void)
	__must_hold(&mutex_)
{
	sendMsg("Session started!", last_msg_id_);
	sendMsg("Shuffling decks...", last_msg_id_);
}

inline void Session::__sendEndMessage(void)
	__must_hold(&mutex_)
{
	sendMsg("Session ended!\n\n" + __generateScoreBoard(), last_msg_id_);
}

inline void Session::__workerFunc(std::unique_lock<std::mutex> &lock)
	__must_hold(&mutex_)
{
	bool first_card = true;

	if (!silent_start_) {
		__sendStartMessage();
		deck_group_.shuffleAllDecks();
		cond_.wait_for(lock, std::chrono::seconds(5));
	}

	__saveToDisk();
	while (!should_stop_) {
		Deck *deck;
		Card *card;

		deck = deck_group_.drawDeck();
		if (!deck)
			break;

		card = deck->drawCard();

		__saveToDisk();
		if (first_card) {
			first_card = false;
		} else {
			/*
			 * Give some delay before sending the next card.
			 */
			cond_.wait_for(lock, std::chrono::seconds(next_card_delay_secs_));
		}

		assert(card);
		if (!__sendCardMayRetry(deck, card, lock, 5))
			break;

		/*
		 * The card has been sent, wait for the answer.
		 */
		__setCurrent(deck, card);
		cond_.wait_for(lock, std::chrono::seconds(timeout_secs_));

		if (current_card_) {
			std::string msg;

			if (should_stop_)
				msg = "Game is stopped.";
			else
				msg = "Time's up!";

			__sendFailMessage(msg);
			__resetCurrent();
		}

		if (should_stop_)
			break;
	}

	__deleteFromDisk();
	__sendEndMessage();
}

void Session::giveSelfPtr(std::shared_ptr<Session> *self_ptr)
{
	std::unique_lock<std::mutex> lock(mutex_);
	assert(!self_ptr_);
	self_ptr_ = self_ptr;
	cond_.notify_one();

	/*
	 * Wait until the worker thread acquires the self_ptr_.
	 */
	while (self_ptr_) {
		if (should_stop_)
			return;

		cond_.wait(lock);
	}
}

/*
 * Keep a shared_ptr to self, so that we can be sure that
 * the object is not destroyed before this detached thread
 * exits.
 */
inline
std::shared_ptr<Session> Session::__waitForSelfPtr(std::unique_lock<std::mutex> &lock)
	__must_hold(&mutex_)
{
	std::shared_ptr<Session> ret;

	/*
	 * Wait until we get the self_ptr_.
	 */
	while (!self_ptr_) {
		if (should_stop_)
			return nullptr;

		pr_debug("Worker thread of %lld is waiting for self_ptr_", (long long)chat_id_);
		cond_.wait(lock);
	}

	ret = *self_ptr_;
	assert(ret);
	self_ptr_ = nullptr;
	pr_debug("Worker thread of %lld acquired self_ptr_", (long long)chat_id_);
	cond_.notify_one();
	return ret;
}

void Session::workerFunc(void)
{
	std::unique_lock<std::mutex> lock(mutex_);
	std::shared_ptr<Session> self;

	try {
		self = __waitForSelfPtr(lock);
		__workerFunc(lock);
	} catch (const std::exception &e) {
		pr_debug("Session worker thread failed: %s", e.what());
		sendMsg("Session worker thread error: " + std::string(e.what()), last_msg_id_);
	}

	should_stop_ = true;
}

void Session::start(bool silent)
{
	std::lock_guard<std::mutex> lock(mutex_);

	try {
		loadAllPointsFromDisk(chat_id_, points_);
		silent_start_ = silent;
		thread_ = std::thread(&Session::workerFunc, this);
		thread_.detach();
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to start session worker thread: " + std::string(e.what()));
	}
}

void Session::stop(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	should_stop_ = true;
	cond_.notify_one();
}

Point Session::__tryLoadPointFromDisk(uint64_t user_id, uint64_t err_reply_to)
	__must_hold(&mutex_)
{
	try {
		return Point::tryLoadFromDisk(chat_id_, user_id);
	} catch (const std::exception &e) {
		pr_debug("Failed to load point from disk, chat_id=%lld, user_id=%lld: %s",
			 (long long)chat_id_, (long long)user_id, e.what());

		std::string msg = "Failed to load point from disk: " + std::string(e.what()) +
				  "\n\nYou will be treated as a new player.";
		sendMsg(msg, err_reply_to, false);
		return Point(user_id, 0, "", "", "");
	}
}

uint64_t Session::__handleCorrectAnswerPoint(const TgBot::Message::Ptr &msg)
	__must_hold(&mutex_)
{
	Point *p;

	auto i = points_.find(msg->from->id);
	if (i == points_.end()) {
		Point tmp = __tryLoadPointFromDisk(msg->from->id, msg->messageId);
		points_.emplace(msg->from->id, tmp);
		p = &points_.at(msg->from->id);
	} else {
		p = &i->second;
	}

	p->addPoint(1);

	try {
		p->saveToDisk(chat_id_);
	} catch (const std::exception &e) {
		pr_debug("Failed to save point to disk, chat_id=%lld, user_id=%lld: %s",
			 (long long)chat_id_, (long long)msg->from->id, e.what());
		sendMsg("Failed to save point to disk: " + std::string(e.what()), msg->messageId, false);
	}

	return p->getPoint();
}

bool Session::answer(const TgBot::Message::Ptr &msg)
{
	std::unique_lock<std::mutex> lock(mutex_);
	std::string txt, reply;
	uint64_t point;

	if (!current_card_)
		return false;

	if (msg->chat->id != chat_id_) {
		pr_debug("Session::answer: Something is wrong, chat_id=%lld, expected=%lld", (long long)msg->chat->id, (long long)chat_id_);
		return false;
	}

	txt = msg->text;
	if (txt.empty())
		return false;

	if (!current_card_->checkAnswer(txt))
		return false;

	point = __handleCorrectAnswerPoint(msg);
	reply = "Correct, +1 point!\n\n"
		"Your point is: " + std::to_string(point) + "\n\n" +
		current_card_->getCardDetails();

	sendMsg(reply, msg->messageId, false);
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
	__must_hold(&mutex_)
{
	json j;

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
	__must_hold(&mutex_)
{
	char path[4096];

	snprintf(path, sizeof(path), "./storage/jqftu/sessions/s_%lld.json", (long long)chat_id_);
	remove(path);
}

void Session::__createSessionDir(void)
	__must_hold(&mutex_)
{
	char path[4096];
	int ret;

	snprintf(path, sizeof(path), "./storage/jqftu/sessions");
	ret = mkdir(path, 0755);
	if (ret && errno != EEXIST)
		throw std::runtime_error("Failed to create directory: " + std::string(strerror(errno)) + ": " + std::string(path));
}

inline void Session::__saveToDisk(void)
	__must_hold(&mutex_)
{
	char path[4096];
	size_t len;
	FILE *fp;

	__createSessionDir();
	snprintf(path, sizeof(path), "./storage/jqftu/sessions/s_%lld.json", (long long)chat_id_);
	fp = fopen(path, "wb");
	if (!fp)
		throw std::runtime_error("Failed to open file for writing: " + std::string(strerror(errno)) + ": " + std::string(path));

	try {
		std::string json_str = __toJsonString();
		len = fwrite(json_str.c_str(), 1, json_str.size(), fp);
		if (len != json_str.length())
			throw std::runtime_error("fwrite error: " + std::string(path));

		fclose(fp);
	} catch (const std::exception &e) {
		fclose(fp);
		throw e;
	}
}

inline void Session::__addDeckByName(const std::string &name)
	__must_hold(&mutex_)
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
	sendMsg("The system is rebooted, trying to recover the session...", last_msg_id_);
}

// static
std::shared_ptr<Session> Session::fromJsonString(Muika &m, const std::string &json_str)
{
	json j = json::parse(json_str);

	if (!j.is_object())
		throw std::runtime_error("The JSON is not an object");

	if (!j.contains("chat_id") || !j["chat_id"].is_number_integer())
		throw std::runtime_error("The JSON does not contain integer chat_id");

	if (!j.contains("last_msg_id") || !j["last_msg_id"].is_number_unsigned())
		throw std::runtime_error("The JSON does not contain unsigned integer last_msg_id");

	if (!j.contains("timeout_secs") || !j["timeout_secs"].is_number_unsigned())
		throw std::runtime_error("The JSON does not contain unsigned integer timeout_secs");

	if (!j.contains("next_card_delay_secs") || !j["next_card_delay_secs"].is_number_unsigned())
		throw std::runtime_error("The JSON does not contain unsigned integer next_card_delay_secs");

	if (!j.contains("deck_group") || !j["deck_group"].is_object())
		throw std::runtime_error("The JSON does not contain object deck_group");

	std::shared_ptr<Session> r = std::make_shared<Session>(m, j["chat_id"], j["last_msg_id"]);
	r->setTimeout(j["timeout_secs"]);
	r->setNextCardDelay(j["next_card_delay_secs"]);
	r->deck_group_.fromJson(j["deck_group"]);
	return r;
}

// static
void Session::loadAllPointsFromDisk(int64_t chat_id, std::unordered_map<uint64_t, Point> &points)
{
	char path[4096];
	DIR *dir;

	snprintf(path, sizeof(path), "./storage/jqftu/points/s_%lld", (long long)chat_id);
	dir = opendir(path);
	if (!dir)
		return;

	while (true) {
		struct dirent *ent = readdir(dir);
		uint64_t user_id;
		size_t len;

		if (!ent)
			break;

		if (ent->d_type != DT_REG)
			continue;

		len = strlen(ent->d_name);
		if (len < 5 || strcmp(ent->d_name + len - 5, ".json"))
			continue;

		user_id = strtoull(ent->d_name, NULL, 10);
		if (!user_id)
			continue;

		try {
			pr_debug("Loading point from disk, chat_id=%lld, user_id=%lld",
				 (long long)chat_id, (long long)user_id);
			Point p = Point::tryLoadFromDisk(chat_id, user_id);
			if (p.getUserId() == user_id)
				points.emplace(user_id, p);
		} catch (const std::exception &e) {
			pr_debug("Failed to load point from disk, chat_id=%lld, user_id=%lld: %s",
				 (long long)chat_id, (long long)user_id, e.what());
		}
	}

	closedir(dir);
}

// static
std::string Session::generateScoreBoard(std::unordered_map<uint64_t, Point> &points)
{
	std::vector<std::pair<int64_t, Point>> scores_vec(points.begin(), points.end());
	std::sort(scores_vec.begin(), scores_vec.end(), [](const auto &a, const auto &b) {
		return a.second.getPoint() > b.second.getPoint();
	});
	std::string ret = "";
	uint32_t i = 1;

	for (auto &s: scores_vec) {
		Point *p = &s.second;
		std::string name;
		std::string line;

		name = htmlspecialchars(p->getFullName());
		name = "<a href=\"tg://user?id=" + std::to_string(p->getUserId()) + "\">" + name + "</a>";

		ret += std::to_string(i++) + ". " + name + ": " + std::to_string(s.second.getPoint()) + " point";
		if (s.second.getPoint() > 1)
			ret += "s";

		ret += "\n";
	}

	return ret;
}

// static
std::string Session::generateScoreBoardFromDisk(int64_t chat_id)
{
	std::unordered_map<uint64_t, Point> points;
	loadAllPointsFromDisk(chat_id, points);
	return generateScoreBoard(points);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
