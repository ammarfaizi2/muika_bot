// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/Modules/Jqftu/ModJqftu.hpp>
#include <cassert>
#include <cstdio>

#ifdef __linux__
#include <pthread.h>
#endif

namespace Muika {
namespace Modules {
namespace Jqftu {

Worker::Worker(ModJqftu *mj, uint32_t tid):
	mj_(mj),
	tid_(tid)
{
}

void Worker::stop(void)
{
	if (thread_.joinable())
		thread_.join();
}

Worker::~Worker(void)
{
	stop();
}

static const std::string help_str =
	"<code>/jqftu</code> is a Japanese quiz module for GNU/Weeb.\n\n"
	"<b>Avaliable commands:</b>\n"
	"- <code>/jqftu help</code> to show this message.\n"
	"- <code>/jqftu help start</code> to show more start cmd help.\n"
	"- <code>/jqftu points</code> to show your points.\n"
	"- <code>/jqftu start [deckname]</code> to start a quiz.\n"
	"- <code>/jqftu stop</code> to stop a quiz.\n"
	"- <code>/jqftu scoreboard</code> to show the scoreboard.\n";

static const std::string help_start_str =
	"Say <code>/jqftu start [deckname]</code> to start a quiz (Example: <code>/jqftu start tozai_line</code>).\n\n"
	"List Japanese Station Name Decks:\n"
	"- (T) <code>tozai_line</code> - Tokyo Metro Tozai Line (東西線)\n"
	"- (KK) <code>keikyu_line</code> - Keikyu Main Line (京急本線)\n"
	"- (KS) <code>keisei_line</code> - Keisei Main Line (京成本線)\n"
	"- (JY) <code>yamanote_line</code> - JR Yamanote Line (山手線)\n"
	"- (JB) <code>chuo_sobu_line</code> - JR Chuo-Sobu Line (中央・総武緩行線)\n"
	"- (JC) <code>chuo_line</code> - JR Chuo Line (中央線快速)\n"
	"- (JK) <code>keihin_tohoku_line</code> - JR Keihin-Tohoku Line (京浜東北線)\n"
	"- (JA) <code>saikyo_line</code> - JR Saikyo Line (埼京線)\n\n"
	"List JLPT Kotoba Decks:\n"
	"- (N5) <code>jlpt_n5</code> - JLPT N5 Kotoba\n";

static void handleCmdHelp(TgBot::Bot *b, int64_t chat_id, uint64_t msg_id,
			  const char *arg)
{
	auto rep = std::make_shared<TgBot::ReplyParameters>();
	std::string rt;

	if (arg) {
		if (!strcmp(arg, "start"))
			rt = help_start_str;
		else
			rt = help_str;
	} else {
		rt = help_str;
	}

	rep->messageId = msg_id;
	rep->chatId = chat_id;
	rep->allowSendingWithoutReply = true;
	rep->quotePosition = 0;
	b->getApi().sendMessage(chat_id, rt, nullptr, rep, nullptr, "HTML");
}

inline void Worker::handleCmd(std::unique_ptr<Msg> &msg)
{
	const TgBot::Message::Ptr &o = msg->orig;
	TgBot::Bot *bot = mj_->mb_->getBot();
	const char *cmd;

	if (msg->cmd_args.empty())
		cmd = "help";
	else
		cmd = msg->cmd_args[0].c_str();

	if (!strcmp(cmd, "help")) {
		const char *arg = msg->cmd_args.size() >= 2 ? msg->cmd_args[1].c_str() : nullptr;
		handleCmdHelp(bot, o->chat->id, o->messageId, arg);
		return;
	}
}

inline void Worker::processMsg(std::unique_ptr<Msg> &msg)
{
	switch (msg->type) {
	case JQFTU_MSG_TYPE_CMD:
		handleCmd(msg);
		break;
	default:
		assert(false && "Unknown msg type");
		pr_err("Jqftu: Worker %u received unknown msg type %d", tid_, msg->type);
		break;
	}
}

inline void Worker::popAndProcessMsg(std::unique_lock<std::mutex> &lk)
{
	while (!mj_->msg_queue_.empty()) {
		std::unique_ptr<Msg> msg = std::move(mj_->msg_queue_.front());
		mj_->msg_queue_.pop();
		lk.unlock();
		try {
			processMsg(msg);
		} catch (TgBot::TgException &e) {
			pr_err("Jqftu: Worker %u caught TgException: %s", tid_, e.what());
		} catch (std::exception &e) {
			pr_err("Jqftu: Worker %u caught std::exception: %s", tid_, e.what());
		} catch (...) {
			pr_err("Jqftu: Worker %u caught unknown exception", tid_);
		}
		lk.lock();
	}
}

inline void Worker::run(WaitThreads *wt)
{
	pr_info("Jqftu: Worker %u started", tid_);
	wt->ready();

	{
		std::unique_lock<std::mutex> lk(mj_->cv_mtx_);
		do {
			popAndProcessMsg(lk);
			mj_->waitAndSleep(lk);
		} while (!mj_->should_stop_);
	}

	pr_info("Jqftu: Worker %u stopping", tid_);
}

void Worker::start(WaitThreads *wt)
{
	thread_ = std::thread(&Worker::run, this, wt);
#ifdef __linux__
	char thname[16];
	snprintf(thname, sizeof(thname), "jqftu-w%u", tid_);
	pthread_setname_np(thread_.native_handle(), thname);
#endif
}

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */
