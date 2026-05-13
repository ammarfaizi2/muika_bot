// SPDX-License-Identifier: GPL-2.0-only
#include <muika/Reactor.hpp>

namespace muika {

Reactor::Reactor(void) = default;
Reactor::~Reactor(void) = default;

std::string Reactor::sendMsgText(const std::string &chat_id,
				 const OutboundMessage &om)
{
	(void)chat_id;
	(void)om;
	return "";
}

std::string Reactor::sendMsgPhotoUrl(const std::string &chat_id,
				     const std::string &photo_url,
				     const OutboundMessage &om)
{
	(void)chat_id;
	(void)photo_url;
	(void)om;
	return "";
}

std::string Reactor::sendMsgPhotoFile(const std::string &chat_id,
				      const std::string &photo_file_path,
				      const OutboundMessage &om)
{
	(void)chat_id;
	(void)photo_file_path;
	(void)om;
	return "";
}

std::string Reactor::sendMsgText(const std::string &chat_id,
				 const std::string &text,
				 const std::string &reply_to_msg_id)
{
	OutboundMessage om;
	om.text = text;
	om.reply_to_msg_id = reply_to_msg_id;
	return sendMsgText(chat_id, om);
}

std::string Reactor::sendMsgPhotoUrl(const std::string &chat_id,
				     const std::string &photo_url,
				     const std::string &caption,
				     const std::string &reply_to_msg_id)
{
	OutboundMessage om;
	om.text = caption;
	om.reply_to_msg_id = reply_to_msg_id;
	return sendMsgPhotoUrl(chat_id, photo_url, om);
}

std::string Reactor::sendMsgPhotoFile(const std::string &chat_id,
				      const std::string &photo_file_path,
				      const std::string &caption,
				      const std::string &reply_to_msg_id)
{
	OutboundMessage om;
	om.text = caption;
	om.reply_to_msg_id = reply_to_msg_id;
	return sendMsgPhotoFile(chat_id, photo_file_path, om);
}

} /* namespace muika */
