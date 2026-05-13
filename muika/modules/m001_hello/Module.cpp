// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m001_hello/Module.hpp>
#include <muika/helpers.hpp>

#include <cctype>

namespace muika {
namespace modules {
namespace m001_hello {

Module::Module(void):
	muika::Module("hello", "A simple module that says hello")
{
}

static bool isHelloCmd(const std::string &text)
{
	if (text.length() < 6)
		return false;

	char first = text[0];
	if (first != '/' && first != '!' && first != '.')
		return false;

	std::string head = text.substr(1, 5);
	mk_strtolower(head);
	if (head != "hello")
		return false;

	return text.length() == 6 ||
	       std::isspace(static_cast<unsigned char>(text[6]));
}

Module::Type Module::passMsg(MsgPtr msg)
{
	if (msg->is_callback())
		return Type::MSG_SKIP;
	if (msg->content().type() != MessageContent::Type::Text)
		return Type::MSG_SKIP;
	if (!isHelloCmd(msg->content().text()))
		return Type::MSG_SKIP;

	std::string txt = "Hello, " + msg->user().name() + "!";
	reactor()->sendMsgText(msg->chat_id(), txt, msg->id());
	return Type::MSG_HANDLED;
}

} /* namespace m001_hello */
} /* namespace modules */
} /* namespace muika */
