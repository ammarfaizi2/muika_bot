// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__OUTBOUND_MESSAGE_HPP
#define MUIKA__OUTBOUND_MESSAGE_HPP

#include <string>
#include <vector>

namespace muika {

struct InlineButton {
	std::string text;
	std::string callback_data;
	std::string url;
};

using InlineKeyboard = std::vector<std::vector<InlineButton>>;

struct OutboundMessage {
	std::string text;
	std::string parse_mode;
	std::string reply_to_msg_id;
	bool disable_web_page_preview = false;
	InlineKeyboard inline_keyboard;
};

} /* namespace muika */

#endif /* MUIKA__OUTBOUND_MESSAGE_HPP */
