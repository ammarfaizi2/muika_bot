// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m001_hello/Module.hpp>

namespace muika {
namespace modules {
namespace m001_hello {

Module::Module(void):
	muika::Module("hello", "A simple module that says hello")
{
}

Module::Type Module::passMsg(MsgPtr msg)
{
	std::string txt = "Hello, " + msg->user().name() + "!";
	mk()->reactor()->sendMsgText(msg->chat_id(), txt);
	return Type::MSG_HANDLED;
}

} /* namespace m001_hello */
} /* namespace modules */
} /* namespace muika */
