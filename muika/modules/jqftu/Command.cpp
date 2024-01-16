// SPDX-License-Identifier: GPL-2.0-only

#include "Command.hpp"
#include "Session.hpp"

#include <mutex>
#include <cctype>
#include <cassert>
#include <unordered_map>

#include "internal.hpp"

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
	"- (JA) <code>saikyo_line</code> - JR Saikyo Line (埼京線)";

Command::Command(Muika &m, TgBot::Message::Ptr &msg):
	m_(m),
	msg_(msg)
{
}

void Command::parseArgs(void)
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
	m_.getBot().getApi().sendMessage(
		msg_->chat->id,
		help_str,
		true,
		msg_->messageId,
		nullptr,
		"HTML"
	);
}

void Command::start(void)
{
	std::unique_lock<std::mutex> lock(sessions_mutex);
	Session *sess;

	if (sessions.find(msg_->chat->id) != sessions.end()) {
		lock.unlock();
		m_.getBot().getApi().sendMessage(
			msg_->chat->id,
			"Another quiz is running. Please finish it first.",
			true,
			msg_->messageId
		);
		return;
	}

	sess = new Session(m_, msg_->chat->id, args_[0]);
	sessions[msg_->chat->id] = sess;
	if (sess->start() < 0) {
		lock.unlock();
		m_.getBot().getApi().sendMessage(
			msg_->chat->id,
			"Failed to start quiz.",
			true,
			msg_->messageId
		);
		delete sess;
		sessions.erase(msg_->chat->id);
	} else {
		lock.unlock();
		m_.getBot().getApi().sendMessage(
			msg_->chat->id,
			"Quiz started!",
			true,
			msg_->messageId
		);
	}
}

void Command::execute(void)
{
	assert(msg_->text.length() >= 6);
	parseArgs();

	if (args_.size() == 0) {
		showHelp();
		return;
	}

	start();
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
