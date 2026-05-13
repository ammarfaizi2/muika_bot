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

#include <nlohmann/json.hpp>

#include <cassert>
#include <cctype>
#include <cerrno>
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
	"- <code>/jqftu scoreboard</code> to show the scoreboard.\n"
	"- <code>/jqftu set &lt;key&gt; &lt;value&gt;</code> to set a config value.\n"
	"- <code>/jqftu help set</code> to show more set cmd help.\n";

const std::string help_set_str =
	"Say <code>/jqftu set &lt;key&gt; &lt;value&gt;</code> to set a config "
	"value.\n\n"
	"<b>Per-chat default keys</b> (applied to newly started sessions):\n"
	"- <code>default.timeout</code> - card timeout in seconds.\n"
	"- <code>default.delay</code> - delay between cards in seconds.\n\n"
	"<b>Current session keys</b> (only when a quiz is running):\n"
	"- <code>cur.timeout</code> - card timeout in seconds.\n"
	"- <code>cur.delay</code> - delay between cards in seconds.\n\n"
	"Examples:\n"
	"- <code>/jqftu set default.timeout 3600</code>\n"
	"- <code>/jqftu set default.delay 5</code>\n"
	"- <code>/jqftu set cur.timeout 1200</code>\n"
	"- <code>/jqftu set cur.delay 300</code>\n";

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

struct ChatConfig {
	bool has_timeout = false;
	bool has_delay = false;
	uint32_t timeout_secs = 0;
	uint32_t next_card_delay_secs = 0;
};

std::string chatConfigPath(const Paths &paths, int64_t chat_id)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "/s_%lld.json", (long long)chat_id);
	return paths.config_dir + buf;
}

ChatConfig loadChatConfig(const Paths &paths, int64_t chat_id)
{
	ChatConfig cfg;
	std::string contents;
	std::string path = chatConfigPath(paths, chat_id);

	if (mk_file_get_contents(path, contents) < 0)
		return cfg;

	try {
		auto j = ::nlohmann::json::parse(contents);
		if (!j.is_object())
			return cfg;

		if (j.contains("timeout_secs") &&
		    j["timeout_secs"].is_number_unsigned()) {
			cfg.timeout_secs = j["timeout_secs"].get<uint32_t>();
			cfg.has_timeout = true;
		}
		if (j.contains("next_card_delay_secs") &&
		    j["next_card_delay_secs"].is_number_unsigned()) {
			cfg.next_card_delay_secs =
				j["next_card_delay_secs"].get<uint32_t>();
			cfg.has_delay = true;
		}
	} catch (const std::exception &) {
	}

	return cfg;
}

void saveChatConfig(const Paths &paths, int64_t chat_id, const ChatConfig &cfg)
{
	if (mk_mkdir_p(paths.config_dir.c_str(), 0755) < 0)
		throw std::runtime_error("Failed to create config dir: " +
					 paths.config_dir);

	::nlohmann::json j = ::nlohmann::json::object();
	if (cfg.has_timeout)
		j["timeout_secs"] = cfg.timeout_secs;
	if (cfg.has_delay)
		j["next_card_delay_secs"] = cfg.next_card_delay_secs;

	std::string path = chatConfigPath(paths, chat_id);
	if (mk_file_put_contents(path, j.dump(1, '\t')) < 0)
		throw std::runtime_error("Failed to write config file: " + path);
}

class SessionMap {
public:
	SessionMap(void) = default;
	~SessionMap(void) = default;

	std::shared_ptr<Session> __find(int64_t chat_id);
	bool __remove(int64_t chat_id);
	void __insert(int64_t chat_id, std::shared_ptr<Session> session);

	void loadSessionsFromDisk(muika::Module *mod, WorkerPool *pool, const Paths &paths);
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

void SessionMap::loadSessionsFromDisk(muika::Module *mod, WorkerPool *pool, const Paths &paths)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const std::string &dir = paths.sessions_dir;

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
			sess = Session::fromJsonString(mod, pool, paths, contents);
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
	Command(muika::Module *mod, MsgPtr msg, SessionMap *smap, WorkerPool *pool, const Paths &paths);
	bool execute(void);

private:
	muika::Module *mod_;
	MsgPtr msg_;
	SessionMap *smap_;
	WorkerPool *pool_;
	Paths paths_;
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
	void set(void);
};

Command::Command(muika::Module *mod, MsgPtr msg, SessionMap *smap, WorkerPool *pool, const Paths &paths):
	mod_(mod),
	msg_(std::move(msg)),
	smap_(smap),
	pool_(pool),
	paths_(paths),
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
	else if (args_.size() == 2 && args_[1] == "set")
		sendMsg(help_set_str);
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
		s = Session::generateScoreBoardFromDisk(paths_.points_dir, chat_id_);
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
			p = Point::tryLoadFromDisk(paths_.points_dir, chat_id_, user_id_);
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
		sess = std::make_shared<Session>(mod_, pool_, paths_, chat_id_, msg_id_);

		ChatConfig ccfg = loadChatConfig(paths_, chat_id_);
		if (ccfg.has_timeout)
			sess->setTimeout(ccfg.timeout_secs);
		if (ccfg.has_delay)
			sess->setNextCardDelay(ccfg.next_card_delay_secs);

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

void Command::set(void)
{
	if (args_.size() != 3) {
		sendMsg("Invalid set command. Say <code>/jqftu help set</code> "
			"for usage.");
		return;
	}

	const std::string &key = args_[1];
	const std::string &val = args_[2];

	if (key != "default.timeout" && key != "default.delay" &&
	    key != "cur.timeout" && key != "cur.delay") {
		sendMsg("Unknown set key: <code>" + key + "</code>. "
			"Say <code>/jqftu help set</code> for usage.");
		return;
	}

	if (val.empty() || val.length() > 10) {
		sendMsg("Invalid value for <code>" + key + "</code>: "
			"must be a positive integer (seconds).");
		return;
	}
	for (char c : val) {
		if (c < '0' || c > '9') {
			sendMsg("Invalid value for <code>" + key + "</code>: "
				"must be a positive integer (seconds).");
			return;
		}
	}

	errno = 0;
	unsigned long secs_ul = strtoul(val.c_str(), nullptr, 10);
	if (errno != 0 || secs_ul == 0 || secs_ul > 86400) {
		sendMsg("Invalid value for <code>" + key + "</code>: "
			"must be between 1 and 86400 seconds.");
		return;
	}
	uint32_t secs = static_cast<uint32_t>(secs_ul);

	if (key == "default.timeout" || key == "default.delay") {
		ChatConfig ccfg;
		try {
			ccfg = loadChatConfig(paths_, chat_id_);
		} catch (const std::exception &e) {
			sendMsg("Failed to load config: " +
				std::string(e.what()));
			return;
		}

		if (key == "default.timeout") {
			ccfg.timeout_secs = secs;
			ccfg.has_timeout = true;
		} else {
			ccfg.next_card_delay_secs = secs;
			ccfg.has_delay = true;
		}

		try {
			saveChatConfig(paths_, chat_id_, ccfg);
		} catch (const std::exception &e) {
			sendMsg("Failed to save config: " +
				std::string(e.what()));
			return;
		}

		sendMsg("Set <code>" + key + "</code> to <code>" +
			std::to_string(secs) + "</code>.");
		return;
	}

	/* cur.* */
	if (!loadSession()) {
		sendMsg("No quiz is running. <code>cur.*</code> keys require "
			"an active session.");
		return;
	}

	if (key == "cur.timeout")
		sess_->setTimeout(secs);
	else
		sess_->setNextCardDelay(secs);

	sendMsg("Set <code>" + key + "</code> to <code>" +
		std::to_string(secs) + "</code>.");
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
	else if (cmd == "set")
		set();
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
		paths_.decks_dir = m_->storageDir() + "/decks";
		paths_.points_dir = m_->storageDir() + "/points";
		paths_.sessions_dir = m_->storageDir() + "/sessions";
		paths_.config_dir = m_->storageDir() + "/config";

		mk_mkdir_p(paths_.points_dir.c_str(), 0755);
		mk_mkdir_p(paths_.sessions_dir.c_str(), 0755);
		mk_mkdir_p(paths_.config_dir.c_str(), 0755);

		std::size_t nr = WorkerPool::workerCountFromEnv(4);
		pool_ = std::make_unique<WorkerPool>(nr);
		smap_ = std::make_unique<SessionMap>();

		smap_->loadSessionsFromDisk(m_, pool_.get(), paths_);
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

		Command cmd(m_, msg, smap_.get(), pool_.get(), paths_);
		bool handled = cmd.execute();
		return handled ? muika::Module::Type::MSG_HANDLED
			       : muika::Module::Type::MSG_SKIP;
	}

	WorkerPool *pool(void) { return pool_.get(); }

private:
	Module *m_;
	Paths paths_;
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
