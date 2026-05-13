// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Module.hpp>
#include <muika/modules/m002_jqftu/Session.hpp>
#include <muika/modules/m002_jqftu/WorkerPool.hpp>
#include <muika/modules/m002_jqftu/Deck.hpp>
#include <muika/modules/m002_jqftu/DeckGroup.hpp>
#include <muika/modules/m002_jqftu/Point.hpp>
#include <muika/modules/m002_jqftu/internal.hpp>
#include <muika/Muika.hpp>
#include <muika/helpers.hpp>

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <dirent.h>

namespace muika {
namespace modules {
namespace m002_jqftu {

namespace {

const std::string help_str =
	"<code>/jqftu</code> is a Japanese quiz module.\n\n"
	"<b>Available commands:</b>\n"
	"- <code>/jqftu help</code> to show this message.\n"
	"- <code>/jqftu help start</code> to show more start cmd help.\n"
	"- <code>/jqftu points</code> to show your points.\n"
	"- <code>/jqftu start [deckname...]</code> to start a quiz.\n"
	"- <code>/jqftu stop</code> to stop a quiz.\n"
	"- <code>/jqftu scoreboard</code> to show the scoreboard.\n";

const std::string help_start_str =
	"Say <code>/jqftu start [deckname]</code> to start a quiz "
	"(Example: <code>/jqftu start tozai_line</code>).\n\n"
	"List Japanese Station Name Decks:\n"
	"- (T) <code>tozai_line</code> - Tokyo Metro Tozai Line (東西線)\n"
	"- (KK) <code>keikyu_line</code> - Keikyu Main Line (京急本線)\n"
	"- (JY) <code>yamanote_line</code> - JR Yamanote Line (山手線)\n"
	"- (JB) <code>chuo_sobu_line</code> - JR Chuo-Sobu Line (中央・総武緩行線)\n\n"
	"List JLPT Kotoba Decks:\n"
	"- (N5) <code>jlpt_n5</code> - JLPT N5 Kotoba\n";

class SessionMap {
public:
	SessionMap(void) = default;
	~SessionMap(void) = default;

	std::shared_ptr<Session> __find(int64_t chat_id);
	bool __remove(int64_t chat_id);
	void __insert(int64_t chat_id, std::shared_ptr<Session> session);

	void loadSessionsFromDisk(muika::Module *mod, WorkerPool *pool);
	void stopAll(void);

	inline std::mutex &getMutex(void) { return mutex_; }

private:
	std::mutex mutex_;
	std::unordered_map<int64_t, std::shared_ptr<Session>> sessions_;
};

std::shared_ptr<Session> SessionMap::__find(int64_t chat_id)
{
	auto it = sessions_.find(chat_id);
	if (it == sessions_.end())
		return nullptr;
	return it->second;
}

bool SessionMap::__remove(int64_t chat_id)
{
	auto it = sessions_.find(chat_id);
	if (it == sessions_.end())
		return false;
	sessions_.erase(it);
	return true;
}

void SessionMap::__insert(int64_t chat_id, std::shared_ptr<Session> session)
{
	sessions_.emplace(chat_id, std::move(session));
}

void SessionMap::stopAll(void)
{
	std::lock_guard<std::mutex> lock(mutex_);
	for (auto &kv : sessions_)
		kv.second->stop();
	sessions_.clear();
}

void SessionMap::loadSessionsFromDisk(muika::Module *mod, WorkerPool *pool)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const std::string &dir = Session::getSessionsDir();

	DIR *d = opendir(dir.c_str());
	if (!d)
		return;

	while (true) {
		struct dirent *ent = readdir(d);
		if (!ent)
			break;

		size_t len = strlen(ent->d_name);
		if (len < 5 || strcmp(ent->d_name + len - 5, ".json"))
			continue;

		std::string path = dir + "/" + ent->d_name;
		std::string contents;
		if (mk_file_get_contents(path, contents) < 0) {
			pr_debug("Failed to read session file %s", path.c_str());
			continue;
		}

		std::shared_ptr<Session> sess;
		try {
			sess = Session::fromJsonString(mod, pool, contents);
		} catch (const std::exception &e) {
			pr_debug("Failed to parse session file %s: %s",
				 path.c_str(), e.what());
			continue;
		}

		try {
			sess->sendRebootMessage();
			sess->start(true);
		} catch (const std::exception &e) {
			pr_debug("Failed to start session %s: %s",
				 path.c_str(), e.what());
			sess->stop();
			continue;
		}

		try {
			__insert(sess->getChatId(), sess);
		} catch (const std::exception &e) {
			pr_debug("Failed to insert session %s: %s",
				 path.c_str(), e.what());
			sess->stop();
		}
	}

	closedir(d);
}

class Command {
public:
	Command(muika::Module *mod, MsgPtr msg, SessionMap *smap, WorkerPool *pool);
	bool execute(void);

private:
	muika::Module *mod_;
	MsgPtr msg_;
	SessionMap *smap_;
	WorkerPool *pool_;
	std::unique_lock<std::mutex> smap_lock_;
	std::shared_ptr<Session> sess_;
	std::vector<std::string> args_;
	int64_t chat_id_;
	uint64_t msg_id_;
	uint64_t user_id_;

	uint64_t sendMsg(const std::string &text, uint64_t reply_to = 0);
	bool parseCmd(void);
	void parseArgs(void);
	bool __loadSession(void);
	bool loadSession(void);
	bool checkAnswerAttempt(void);
	void sendInvalidCommandResponse(void);
	void help(void);
	void scoreboard(void);
	void points(void);
	void start(void);
	void stop(void);
};

Command::Command(muika::Module *mod, MsgPtr msg, SessionMap *smap, WorkerPool *pool):
	mod_(mod),
	msg_(std::move(msg)),
	smap_(smap),
	pool_(pool),
	smap_lock_(smap->getMutex(), std::defer_lock)
{
	chat_id_ = strtoll(msg_->chat_id().c_str(), nullptr, 10);
	msg_id_  = strtoull(msg_->id().c_str(), nullptr, 10);
	user_id_ = strtoull(msg_->user().id().c_str(), nullptr, 10);
}

uint64_t Command::sendMsg(const std::string &text, uint64_t reply_to)
{
	if (!reply_to)
		reply_to = msg_id_;

	OutboundMessage om;
	om.text = text;
	om.parse_mode = "HTML";
	if (reply_to)
		om.reply_to_msg_id = std::to_string(reply_to);

	std::string id = mod_->reactor()->sendMsgText(msg_->chat_id(), om);
	return id.empty() ? 0 : strtoull(id.c_str(), nullptr, 10);
}

bool Command::parseCmd(void)
{
	const std::string &text = msg_->content().text();
	if (text.length() < 6)
		return false;

	char first = text[0];
	if (first != '.' && first != '/' && first != '!')
		return false;

	std::string head = text.substr(1, 5);
	mk_strtolower(head);
	if (head != "jqftu")
		return false;

	if (text.length() == 6)
		return true;

	return std::isspace(static_cast<unsigned char>(text[6]));
}

void Command::parseArgs(void)
{
	const std::string &text = msg_->content().text();
	const char *start = text.c_str() + 6;
	const char *end = text.c_str() + text.length();

	while (start < end) {
		while (start < end && std::isspace(static_cast<unsigned char>(*start)))
			start++;
		if (start == end)
			break;

		const char *arg_start = start;
		while (start < end && !std::isspace(static_cast<unsigned char>(*start)))
			start++;

		args_.emplace_back(arg_start, start - arg_start);
	}
}

bool Command::__loadSession(void)
{
	if (sess_)
		return true;

	sess_ = smap_->__find(chat_id_);
	if (sess_ && sess_->getShouldStop()) {
		smap_->__remove(chat_id_);
		sess_.reset();
	}

	return sess_ != nullptr;
}

bool Command::loadSession(void)
{
	if (sess_)
		return true;

	smap_lock_.lock();
	bool ret = __loadSession();
	smap_lock_.unlock();
	return ret;
}

bool Command::checkAnswerAttempt(void)
{
	if (loadSession())
		return sess_->answer(msg_);
	return false;
}

void Command::sendInvalidCommandResponse(void)
{
	sendMsg("Invalid command. Say <code>/jqftu help</code> to show help.");
}

void Command::help(void)
{
	if (args_.size() < 2)
		sendMsg(help_str);
	else if (args_.size() == 2 && args_[1] == "start")
		sendMsg(help_start_str);
	else
		sendInvalidCommandResponse();
}

void Command::scoreboard(void)
{
	std::string s;

	smap_lock_.lock();
	if (__loadSession())
		s = sess_->generateScoreBoard();
	else
		s = Session::generateScoreBoardFromDisk(chat_id_);
	smap_lock_.unlock();

	if (s.empty())
		s = "No scoreboard for this chat!";

	sendMsg(s);
}

void Command::points(void)
{
	Point p;

	if (loadSession()) {
		p = sess_->getPoint(user_id_);
	} else {
		try {
			p = Point::tryLoadFromDisk(chat_id_, user_id_);
		} catch (const std::exception &e) {
			sendMsg("Failed to load points: " + std::string(e.what()));
			return;
		}
	}

	sendMsg("<b>Your point is:</b> " + std::to_string(p.getPoint()));
}

void Command::start(void)
{
	smap_lock_.lock();
	if (__loadSession()) {
		smap_lock_.unlock();
		sendMsg("A quiz is already running.");
		return;
	}

	if (args_.size() < 2) {
		smap_lock_.unlock();
		sendMsg(help_start_str);
		return;
	}

	std::shared_ptr<Session> sess;
	try {
		sess = std::make_shared<Session>(mod_, pool_, chat_id_, msg_id_);
		for (size_t i = 1; i < args_.size(); i++)
			sess->addDeckByName(args_[i]);
		sess->start();
	} catch (const std::exception &e) {
		smap_lock_.unlock();
		pr_debug("Cannot start a session: %s", e.what());
		sendMsg("Cannot start a session: " + std::string(e.what()));
		return;
	}

	try {
		smap_->__insert(chat_id_, sess);
	} catch (const std::exception &e) {
		smap_lock_.unlock();
		pr_debug("Cannot insert a session: %s", e.what());
		sendMsg("Cannot insert a session: " + std::string(e.what()));
		sess->stop();
		return;
	}

	smap_lock_.unlock();
}

void Command::stop(void)
{
	smap_lock_.lock();
	if (!__loadSession()) {
		smap_lock_.unlock();
		sendMsg("No quiz is running.");
		return;
	}

	sess_->stop();
	smap_->__remove(chat_id_);
	smap_lock_.unlock();
	sendMsg("Quiz stopped.");
}

bool Command::execute(void)
{
	if (!parseCmd())
		return checkAnswerAttempt();

	parseArgs();
	if (args_.empty()) {
		help();
		return true;
	}

	std::string cmd = args_[0];
	mk_strtolower(cmd);

	if (cmd == "help")
		help();
	else if (cmd == "points" || cmd == "point")
		points();
	else if (cmd == "start")
		start();
	else if (cmd == "stop")
		stop();
	else if (cmd == "scoreboard")
		scoreboard();
	else
		sendInvalidCommandResponse();

	return true;
}

} /* anonymous namespace */

class Internal {
public:
	inline Internal(Module *m): m_(m) {}

	void init(void)
	{
		Deck::setDecksDir(m_->storageDir() + "/decks");
		Point::setPointsDir(m_->storageDir() + "/points");
		Session::setSessionsDir(m_->storageDir() + "/sessions");

		mk_mkdir_p((m_->storageDir() + "/points").c_str(), 0755);
		mk_mkdir_p((m_->storageDir() + "/sessions").c_str(), 0755);

		std::size_t nr = WorkerPool::workerCountFromEnv(4);
		pool_ = std::make_unique<WorkerPool>(nr);
		smap_ = std::make_unique<SessionMap>();

		smap_->loadSessionsFromDisk(m_, pool_.get());
	}

	void free(void)
	{
		if (smap_)
			smap_->stopAll();
		if (pool_)
			pool_->shutdown();
		smap_.reset();
		pool_.reset();
	}

	muika::Module::Type passMsg(MsgPtr msg)
	{
		if (msg->is_callback())
			return muika::Module::Type::MSG_SKIP;
		if (msg->content().type() != MessageContent::Type::Text)
			return muika::Module::Type::MSG_SKIP;
		if (!smap_ || !pool_)
			return muika::Module::Type::MSG_SKIP;

		Command cmd(m_, msg, smap_.get(), pool_.get());
		bool handled = cmd.execute();
		return handled ? muika::Module::Type::MSG_HANDLED
			       : muika::Module::Type::MSG_SKIP;
	}

	WorkerPool *pool(void) { return pool_.get(); }

private:
	Module *m_;
	std::unique_ptr<WorkerPool> pool_;
	std::unique_ptr<SessionMap> smap_;
};

Module::Module(void):
	muika::Module("jqftu", "Japanese Quiz")
{
	internal_ = std::make_unique<Internal>(this);
}

Module::~Module(void) = default;

void Module::init(void)
{
	internal_->init();
}

void Module::free(void)
{
	internal_->free();
}

Module::Type Module::passMsg(MsgPtr msg)
{
	return internal_->passMsg(msg);
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
