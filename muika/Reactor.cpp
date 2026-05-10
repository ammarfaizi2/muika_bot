// SPDX-License-Identifier: GPL-2.0-only
#include <muika/Reactor.hpp>

namespace muika {

Reactor::Reactor(void) = default;
Reactor::~Reactor(void) = default;

void Reactor::sendMsgText(const std::string &chat_id,
			    const std::string &text,
			    const std::string &reply_to_msg_id)
{
	(void)chat_id;
	(void)text;
	(void)reply_to_msg_id;
}

void Reactor::sendMsgPhotoUrl(const std::string &chat_id,
				 const std::string &photo_url,
				 const std::string &caption,
				 const std::string &reply_to_msg_id)
{
	(void)chat_id;
	(void)photo_url;
	(void)caption;
	(void)reply_to_msg_id;
}

void Reactor::sendMsgPhotoFile(const std::string &chat_id,
				  const std::string &photo_file_path,
				  const std::string &caption,
				  const std::string &reply_to_msg_id)
{
	(void)chat_id;
	(void)photo_file_path;
	(void)caption;
	(void)reply_to_msg_id;
}

} /* namespace muika */
