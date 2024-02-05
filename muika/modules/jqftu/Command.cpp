// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Command.hpp"
#include "muika/modules/jqftu/internal.hpp"
#include "muika/modules/jqftu/Point.hpp"
#include "muika/helpers.hpp"

#include <mutex>
#include <cctype>
#include <cassert>
#include <cstring>
#include <unordered_map>

using muika::strtolower;

namespace muika {
namespace modules {
namespace jqftu {

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
	"- (KK) <code>keikyu_line</code> - Keikyu Line (京急線)\n"
	"- (KS) <code>keisei_line</code> - Keisei Line (京成線)\n"
	"- (JY) <code>yamanote_line</code> - JR Yamanote Line (山手線)\n"
	"- (JB) <code>chuo_sobu_line</code> - JR Chuo-Sobu Line (中央・総武緩行線)\n"
	"- (JC) <code>chuo_line</code> - JR Chuo Line (中央線快速)\n"
	"- (JK) <code>keihin_tohoku_line</code> - JR Keihin-Tohoku Line (京浜東北線)\n"
	"- (JA) <code>saikyo_line</code> - JR Saikyo Line (埼京線)\n\n"
	"List JLPT Kotoba Decks:\n"
	"- (N5) <code>jlpt_n5</code> - JLPT N5 Kotoba\n";

Command::Command(Muika &m, const TgBot::Message::Ptr &msg, SessionMap *smap):
	m_(m),
	msg_(msg),
	smap_(smap),
	smap_lock_(smap->getMutex(), std::defer_lock)
{
}

inline uint64_t Command::sendMsg(const std::string &msg, uint64_t reply_to)
{
	if (reply_to == (uint64_t)-1)
		reply_to = msg_->messageId;

	auto i = m_.getApi().sendMessage(msg_->chat->id, msg, false, reply_to, nullptr, "HTML");
	return i->messageId;
}

inline bool Command::parseCmd(void)
{
	std::string text = msg_->text;

	if (text.length() < 6)
		return false;

	text = text.substr(0, 6);
	strtolower(text);
	if (text[0] == '.' || text[0] == '/' || text[0] == '!') {
		if (text.substr(1, 5) == "jqftu") {
			if (text.length() == 6)
				return true;

			if (std::isspace(text[6]))
				return true;
		}
	}

	return false;
}

inline bool Command::__loadSession(void)
	__must_hold(&smap_->mutex_)
{
	if (sess_)
		return true;

	sess_ = smap_->__find(msg_->chat->id);
	if (sess_ && sess_->getShouldStop()) {
		smap_->__remove(msg_->chat->id);
		sess_ = nullptr;
	}

	return (sess_ != nullptr);
}

bool Command::loadSession(void)
{
	bool ret;

	if (sess_)
		return true;

	smap_lock_.lock();
	ret = __loadSession();
	smap_lock_.unlock();
	return ret;
}

bool Command::checkAnswerAttempt(void)
{
	if (loadSession())
		return sess_->answer(msg_);

	return false;
}

inline void Command::parseArgs(void)
{
	/*
	 * The command format is:
	 * /jqftu [command] [arg1] [arg2] [arg3]
	 *
	 * Note that "/jqftu" is 6 characters long.
	 */
	const char *start = msg_->text.c_str() + 6;
	const char *end = msg_->text.c_str() + msg_->text.length();

	while (start < end) {
		const char *arg_start;

		while (start < end && std::isspace(*start))
			start++;

		if (start == end)
			break;

		arg_start = start;
		while (start < end && !std::isspace(*start))
			start++;

		args_.push_back(std::string(arg_start, start - arg_start));
	}
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

inline void Command::sendInvalidCommandResponse(void)
{
	sendMsg("Invalid command. Say <code>/jqftu help</code> to show help.");
}

inline void Command::scoreboard(void)
{
	std::string s;

	smap_lock_.lock();
	if (__loadSession())
		s = sess_->generateScoreBoard();
	else
		s = Session::generateScoreBoardFromDisk(msg_->chat->id);
	smap_lock_.unlock();

	if (s.empty())
		s = "No scoreboard for this chat!";

	sendMsg(s);
}

inline void Command::points(void)
{
	Point p;

	if (loadSession())
		p = sess_->getPoint(msg_->from->id);
	else
		p = Point::tryLoadFromDisk(msg_->from->id, msg_->chat->id);

	sendMsg("<b>Your point is:</b> " + std::to_string(p.getPoint()));
}

inline void Command::start(void)
{
	std::shared_ptr<Session> sess;

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

	try {
		size_t i;

		sess = std::make_shared<Session>(m_, msg_->chat->id, msg_->messageId);
		for (i = 1; i < args_.size(); i++)
			sess->addDeckByName(args_[i]);

		sess->start();
		sess->giveSelfPtr(&sess);
	} catch (const std::exception &e) {
		pr_debug("Cannot start a session: %s", e.what());
		sendMsg("Cannot start a session: " + std::string(e.what()));
	}

	try {
		smap_->__insert(msg_->chat->id, sess);
	} catch (const std::exception &e) {
		pr_debug("Cannot insert a session: %s", e.what());
		sendMsg("Cannot insert a session: " + std::string(e.what()));
		sess->stop();
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
	smap_->__remove(msg_->chat->id);
	smap_lock_.unlock();
}

bool Command::execute(void)
{
	if (!parseCmd())
		return checkAnswerAttempt();

	parseArgs();
	if (args_.empty() || args_[0] == "help") {
		help();
		return true;
	}

	std::string cmd = args_[0];
	strtolower(cmd);

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

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
