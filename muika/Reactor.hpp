// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__REACTOR_HPP
#define MUIKA__REACTOR_HPP

#include <string>

#include <muika/OutboundMessage.hpp>

namespace muika {

class Reactor {
public:
	Reactor(void);
	virtual ~Reactor(void);

	/*
	 * Primary virtuals: take an OutboundMessage so platform
	 * adapters can consume parse_mode, inline keyboards, etc.
	 *
	 * All return the new message id (as a string), or an empty
	 * string on failure / when the platform cannot report one.
	 */
	virtual std::string sendMsgText(const std::string &chat_id,
					const OutboundMessage &om);

	virtual std::string sendMsgPhotoUrl(const std::string &chat_id,
					    const std::string &photo_url,
					    const OutboundMessage &om);

	virtual std::string sendMsgPhotoFile(const std::string &chat_id,
					     const std::string &photo_file_path,
					     const OutboundMessage &om);

	/*
	 * Convenience overloads for callers that only need a plain
	 * text message without parse mode or keyboards.
	 */
	std::string sendMsgText(const std::string &chat_id,
				const std::string &text,
				const std::string &reply_to_msg_id = "");

	std::string sendMsgPhotoUrl(const std::string &chat_id,
				    const std::string &photo_url,
				    const std::string &caption = "",
				    const std::string &reply_to_msg_id = "");

	std::string sendMsgPhotoFile(const std::string &chat_id,
				     const std::string &photo_file_path,
				     const std::string &caption = "",
				     const std::string &reply_to_msg_id = "");
};

} /* namespace muika */

#endif /* #ifndef MUIKA__REACTOR_HPP */
