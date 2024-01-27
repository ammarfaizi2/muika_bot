// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Command.hpp"
#include "muika/modules/jqftu/Session.hpp"
#include "muika/modules/jqftu/internal.hpp"

#include <mutex>
#include <cctype>
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace muika {
namespace modules {
namespace jqftu {

static const std::string help_str =
	"Say <code>/jqftu deckname</code> to start a quiz (Example: <code>/jqftu tozai_line</code>).\n\n"
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

inline void Command::parseArgs(void)
{
	const char *start = msg_->text.c_str() + 6;
	const char *end = msg_->text.c_str() + msg_->text.length();

	while (start < end) {
		while (start < end && std::isspace(*start))
			start++;

		if (start == end)
			break;

		const char *arg_start = start;
		while (start < end && !std::isspace(*start))
			start++;

		args_.push_back(std::string(arg_start, start - arg_start));
	}
}

void Command::showHelp(void)
{
	m_.getBot().getApi().sendMessage(msg_->chat->id, help_str, true,
					 msg_->messageId, nullptr, "HTML");
}

void Command::start(void)
{
	Session *sess;

	sess = Session::createSession(m_, msg_->chat->id, args_[0]);
	if (!sess) {
		m_.getBot().getApi().sendMessage(
			msg_->chat->id, "Failed to start a session!");
		return;
	}

	sess->start();
	Session::putSession(sess);
}

void Command::execute(void)
{
	assert(msg_->text.length() >= 6);
	parseArgs();

	if (args_.size() != 1) {
		showHelp();
		return;
	}

	if (args_[0] == "stop") {
		m_.getBot().getApi().sendMessage(msg_->chat->id, "No session to stop!");
		return;
	}

	start();
}

static std::string strtolower(const std::string &str)
{
	std::string ret;

	for (auto c : str)
		ret += std::tolower(c);

	return ret;
}

bool Command::isStopCommand(void)
{
	if (msg_->text.length() < 6)
		return false;

	parseArgs();

	if (args_.size() != 1)
		return false;

	return strtolower(args_[0]) == "stop";
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
