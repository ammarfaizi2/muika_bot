// SPDX-License-Identifier: GPL-2.0-only
#include <muika/Message.hpp>

namespace muika {

User::User(const std::string &id, const std::string &name,
	   const std::string &uname):
	id_(id),
	name_(name),
	uname_(uname)
{
}

User::~User(void) = default;

MessageContent::MessageContent(const std::string &text):
	type_(Type::Text),
	content_(text)
{
}

MessageContent::MessageContent(Type type, const std::string &payload):
	type_(type),
	content_(payload)
{
}

MessageContent::~MessageContent(void) = default;

Message::Message(const std::string &chat_id, const std::string &id,
		 const User &user, const MessageContent &content):
	chat_id_(chat_id),
	id_(id),
	user_(user),
	content_(content)
{
}

Message::~Message(void) = default;

} /* namespace muika */
