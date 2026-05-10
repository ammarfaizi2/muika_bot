// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MESSAGE_HPP
#define MUIKA__MESSAGE_HPP

#include <string>
#include <variant>

namespace muika {

class User {
public:
	User(const std::string &id, const std::string &name,
	     const std::string &uname);
	~User(void);

	inline const std::string &id(void) const { return id_; }
	inline const std::string &name(void) const { return name_; }
	inline const std::string &uname(void) const { return uname_; }

private:
	std::string id_;
	std::string name_;
	std::string uname_;

	friend class Message;
};

class MessageContent {
public:
	enum class Type {
		Text,
		Image,
		Video,
		Audio,
		Unknown,
	};

	MessageContent(const std::string &text);
	~MessageContent(void);

	inline Type type(void) const { return type_; }
	inline const std::string &text(void) const { return std::get<std::string>(content_); }

private:
	Type type_;
	std::variant<std::string> content_;
	friend class Message;
};

class Message {
public:
	Message(const std::string &chat_id, const std::string &id,
		const User &user, const MessageContent &content);
	~Message(void);

	inline const std::string &id(void) const { return id_; }
	inline const std::string &chat_id(void) const { return chat_id_; }
	inline const User &user(void) const { return user_; }
	inline const MessageContent &content(void) const { return content_; }

private:
	std::string chat_id_;
	std::string id_;
	User user_;
	MessageContent content_;
};

} /* namespace muika */

#endif /* #ifndef MUIKA__MESSAGE_HPP */
