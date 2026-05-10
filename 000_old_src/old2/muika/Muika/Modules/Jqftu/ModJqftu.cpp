// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/Modules/Jqftu/ModJqftu.hpp>
#include <muika/helpers.hpp>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cinttypes>
#include <dirent.h>

namespace Muika {
namespace Modules {
namespace Jqftu {

static const uint32_t num_threads = 4;

ModJqftu::ModJqftu(MuikaBot *mb):
	Module(mb)
{
	setModName("Jqftu");
}

ModJqftu::~ModJqftu(void)
{
	shutdown();
}

int ModJqftu::init(void)
{
	WaitThreads wt;
	uint32_t i;

	loadSessionsFromDisk();
	for (i = 0; i < num_threads; i++) {
		pr_info("Jqftu: Starting worker thread %u", i);
		std::unique_ptr<Worker> w = std::make_unique<Worker>(this, i);
		w->start(&wt);
		workers_.push_back(std::move(w));
	}

	wt.wait(num_threads);
	return 0;
}

void ModJqftu::shutdown(void)
{
	{
		std::unique_lock<std::mutex> lk(cv_mtx_);
		should_stop_ = true;
		cv_.notify_all();
	}

	for (auto &w : workers_)
		w->stop();

	workers_.clear();
}

static bool endsWith(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return false;

	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr)
		return false;

	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

class ScanDir {
public:
	inline ScanDir(const std::string &path):
		path_(path)
	{
	}

	inline ~ScanDir(void)
	{
		if (dir_)
			closedir(dir_);
	}

	inline int open(void)
	{
		dir_ = opendir(path_.c_str());
		return dir_ ? 0 : -errno;
	}

	inline struct dirent *read(void)
	{
		return dir_ ? readdir(dir_) : nullptr;
	}

	DIR *dir_ = nullptr;
	std::string path_;
};

inline void ModJqftu::loadSessionsFromDisk(void)
{
	std::string sess_dir = getSessionDir();
	ScanDir sd(sess_dir);
	int err;

	err = sd.open();
	if (err) {
		if (err != -ENOENT) {
			pr_err("Jqftu: Failed to open directory '%s': %s",
				jqftu_dir_.c_str(), strerror(-err));
			return;
		}

		err = mkdir_p(jqftu_dir_.c_str(), 0755);
		if (err) {
			pr_err("Jqftu: Failed to create directory '%s': %s",
				jqftu_dir_.c_str(), strerror(-err));
			return;
		}
	}

	while (1) {
		struct dirent *de = sd.read();
		if (!de)
			break;

		if (de->d_type != DT_REG)
			continue;

		if (!endsWith(de->d_name, ".json"))
			continue;

		std::string path = sess_dir + "/" + de->d_name;
		pr_info("Jqftu: Loading session from file '%s'", path.c_str());
		std::unique_ptr<Session> s = Session::loadFromFile(path);
		if (!s) {
			pr_err("Jqftu: Failed to load session from file '%s'",
				path.c_str());
			continue;
		}

		int64_t chat_id = s->getChatId();
		pr_info("Jqftu: Loaded session for chat ID %" PRId64, chat_id);
		sess_map_[chat_id] = std::move(s);
	}
}

static bool isMsgJqftuCmd(const char *txt, std::vector<std::string> *cmd_args)
{
	static const char prefixes[] = ".!/#";
	bool is_cmd = false;
	char c = txt[0];
	size_t i;

	for (i = 0; i < sizeof(prefixes) - 1; i++) {
		if (c == prefixes[i]) {
			is_cmd = true;
			break;
		}
	}

	if (!is_cmd)
		return false;

	txt++;
	if (strcmpi(txt, "jqftu", 5))
		return false;

	if (strncmpi(txt, "jqftu", 5))
		return false;

	txt += 5;
	if (*txt && *txt != ' ' && *txt != '\t' && *txt != '\n')
		return false;

	cmd_args->clear();
	while (*txt) {
		while (*txt == ' ' || *txt == '\t' || *txt == '\n')
			txt++;

		if (!*txt)
			break;

		const char *start = txt;
		while (*txt && *txt != ' ' && *txt != '\t' && *txt != '\n')
			txt++;
		cmd_args->emplace_back(start, txt - start);
	}

	return true;
}

std::shared_ptr<Session> ModJqftu::__getSession(int64_t chat_id)
{
	auto it = sess_map_.find(chat_id);
	if (it != sess_map_.end())
		return it->second;

	return nullptr;
}

std::shared_ptr<Session> ModJqftu::__createSession(int64_t chat_id)
{
	std::shared_ptr<Session> s = __getSession(chat_id);
	if (s)
		return nullptr;

	s = std::make_shared<Session>(this);
	sess_map_[chat_id] = s;
	return s;
}

int ModJqftu::__deleteSession(int64_t chat_id)
{
	auto it = sess_map_.find(chat_id);
	if (it == sess_map_.end())
		return -1;

	pr_info("Jqftu: Deleting session for chat ID %" PRId64, chat_id);
	sess_map_.erase(it);

	std::string path = getSessionDir() + "/" + std::to_string(chat_id) + ".json";
	pr_info("Jqftu: Deleting session file '%s'", path.c_str());
	if (remove(path.c_str())) {
		int err = errno;
		pr_err("Jqftu: Failed to delete session file '%s': %s",
			path.c_str(), strerror(err));
	}
	return 0;
}

std::shared_ptr<Session> ModJqftu::getSession(int64_t chat_id)
{
	std::lock_guard<std::mutex> lk(sess_mtx_);
	return __getSession(chat_id);
}

std::shared_ptr<Session> ModJqftu::createSession(int64_t chat_id)
{
	std::lock_guard<std::mutex> lk(sess_mtx_);
	return __createSession(chat_id);
}

int ModJqftu::deleteSession(int64_t chat_id)
{
	std::lock_guard<std::mutex> lk(sess_mtx_);
	return __deleteSession(chat_id);
}

inline std::unique_ptr<Msg> ModJqftu::constructMsg(const TgBot::Message::Ptr &msg)
{
	std::vector<std::string> cmd_args;
	std::shared_ptr<Session> sess;
	std::string stxt = msg->text;
	std::unique_ptr<Msg> m;

	sess = getSession(msg->chat->id);
	m = std::make_unique<Msg>(msg, sess);
	if (isMsgJqftuCmd(stxt.c_str(), &cmd_args)) {
		m->setCmd(std::move(cmd_args));
		return m;
	}

	if (sess) {
		m->setAnswer();
		return m;
	}

	return nullptr;
}

int ModJqftu::invoke(TgBot::Message::Ptr &msg)
{
	std::unique_ptr<Msg> m = constructMsg(msg);
	if (!m)
		return MOD_INVOKE_SKIP;

	std::unique_lock<std::mutex> lk(cv_mtx_);
	msg_queue_.push(std::move(m));
	if (nr_sleeping_)
		cv_.notify_one();
	return MOD_INVOKE_CONTINUE;
}

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */
