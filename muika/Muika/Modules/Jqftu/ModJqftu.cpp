// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/Modules/Jqftu/ModJqftu.hpp>
#include <muika/helpers.hpp>

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

inline std::unique_ptr<Msg> ModJqftu::constructMsg(const TgBot::Message::Ptr &msg)
{
	std::vector<std::string> cmd_args;
	std::string stxt = msg->text;
	std::unique_ptr<Msg> m;

	if (isMsgJqftuCmd(stxt.c_str(), &cmd_args)) {
		m = std::make_unique<Msg>(msg);
		m->setCmd(std::move(cmd_args));
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
	return MOD_INVOKE_OK;
}

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */
