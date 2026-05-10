// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__REACTOR_HPP
#define MUIKA__REACTOR_HPP

#include <string>

namespace muika {

class Reactor {
public:
	Reactor(void);
	~Reactor(void);

	virtual void sendMsgText(const std::string &chat_id,
				 const std::string &text,
				 const std::string &reply_to_msg_id = "") = 0;

	virtual void sendMsgPhotoUrl(const std::string &chat_id,
				     const std::string &photo_url,
				     const std::string &caption = "",
				     const std::string &reply_to_msg_id = "") = 0;

	virtual void sendMsgPhotoFile(const std::string &chat_id,
				      const std::string &photo_file_path,
				      const std::string &caption = "",
				      const std::string &reply_to_msg_id = "") = 0;
};

} /* namespace muika */

#endif /* #ifndef MUIKA__REACTOR_HPP */
