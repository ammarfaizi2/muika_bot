// SPDX-License-Identifier: GPL-2.0-only
#ifndef MTGBOT__REACTOR_HPP
#define MTGBOT__REACTOR_HPP

#include <muika/Muika.hpp>
#include <mtgbot/Bot.hpp>

namespace TgBot {
class Bot;
class HttpClient;
} /* namespace TgBot */

namespace mtgbot {

class Bot;

class Reactor: public muika::Reactor {
public:
	Reactor(Bot *bot);
	virtual ~Reactor(void);
	virtual void sendMsgText(const std::string &chat_id,
				 const std::string &text,
				 const std::string &reply_to_msg_id = "") override;
	virtual void sendMsgPhotoUrl(const std::string &chat_id,
				     const std::string &photo_url,
				     const std::string &caption = "",
				     const std::string &reply_to_msg_id = "") override;
	virtual void sendMsgPhotoFile(const std::string &chat_id,
				      const std::string &photo_file_path,
				      const std::string &caption = "",
				      const std::string &reply_to_msg_id = "") override;

private:
	Bot *bot_;
	TgBot::Bot *bot(void);
};

} /* namespace mtgbot */

#endif /* MTGBOT__REACTOR_HPP */
