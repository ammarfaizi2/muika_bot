// SPDX-License-Identifier: GPL-2.0-only

#include "Jqftu.hpp"
#include <cassert>
#include <cctype>

namespace muika {

Jqftu::Jqftu(Muika &m, TgBot::Message::Ptr &msg):
	m_(m),
	msg_(msg)
{
}

Jqftu::~Jqftu(void)
{
}

inline void Jqftu::parseArgs(void)
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

inline void Jqftu::showHelp(void)
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

int Jqftu::execute(void)
{
	assert(msg_->text.length() >= 6);
	parseArgs();

	if (args_.empty()) {
		showHelp();
		return 0;
	}

	return 0;
}

} /* namespace muika */
