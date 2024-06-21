// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/Command.hpp>
#include <muika/helpers.hpp>

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
	"- <code>/jqftu scoreboard</code> to show the scoreboard.\n"
	"- <code>/jqftu set_ncd [seconds]</code> to set the delay between the next card.\n"
	"- <code>/jqftu set_timeout [seconds]</code> to set the timeout for answering a question.\n";

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

Command::Command(Muika &m, TgBot::Message::Ptr &msg, SessionMap &smap):
	m_(m),
	msg_(msg),
	smap_(smap),
	cmd_(""),
	args_({})
{
}

Command::~Command(void) = default;

inline
uint64_t Command::sendMsg(const std::string &msg, uint64_t reply_to)
{
	if (reply_to == (uint64_t)-1)
		reply_to = msg_->messageId;

	auto i = m_.getApi().sendMessage(msg_->chat->id, msg, false, reply_to, nullptr, "HTML");
	return i->messageId;
}

inline
bool Command::parseCommand(void)
{
	static const char cmd_prefixes[] = { '/', '!', '.', '$', '?' };
	const std::string &text = msg_->text;
	size_t i;
	char t;

	if (text.empty())
		return false;

	t = text[0];
	for (i = 0; i < sizeof(cmd_prefixes); i++) {
		if (t == cmd_prefixes[i])
			break;
	}

	if (i == sizeof(cmd_prefixes))
		return false;

	std::vector<std::string> parts = str_explode(text, " ", 3);

	cmd_ = parts[0].substr(1);
	if (cmd_ != "jqftu")
		return false;

	if (parts.size() > 1)
		sub_cmd_ = parts[1];

	if (parts.size() > 2)
		args_ = str_explode(parts[2], " ");

	return true;
}

inline
bool Command::handleCommand(void)
{
	if (sub_cmd_ == "start")
		return handleCmdStart();
	else if (sub_cmd_ == "stop")
		return handleCmdStop();
	else if (sub_cmd_ == "help")
		return handleCmdHelp();
	else if (sub_cmd_ == "points")
		return handleCmdPoints();
	else if (sub_cmd_ == "scoreboard")
		return handleCmdScoreboard();
	else if (sub_cmd_ == "set_timeout")
		return handleCmdSetTimeout();
	else if (sub_cmd_ == "set_ncd")
		return handleCmdSetNextCardDelay();
	else
		return handleUnknownCommand();
}

inline
bool Command::handleCmdStart(void)
{
	return true;
}

inline
bool Command::handleCmdStop(void)
{
	return true;
}

inline
bool Command::handleCmdHelp(void)
{
	if (args_.size() < 2)
		sendMsg(help_str);
	else if (args_.size() == 2 && args_[1] == "start")
		sendMsg(help_start_str);
	else
		handleUnknownCommand();

	return true;
}

inline
bool Command::handleCmdPoints(void)
{
	return true;
}

inline
bool Command::handleUnknownCommand(void)
{
	sendMsg("Invalid command. Send <code>/jqftu help</code> to show help.");
	return true;
}

inline
bool Command::handleCmdScoreboard(void)
{
	return true;
}

bool Command::handleCmdSetTimeout(void)
{
	return true;
}

bool Command::handleCmdSetNextCardDelay(void)
{
	return true;
}

bool Command::handle(void)
{
	if (parseCommand())
		return handleCommand();

	return false;
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
