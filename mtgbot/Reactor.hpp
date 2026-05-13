// SPDX-License-Identifier: GPL-2.0-only
#ifndef MTGBOT__REACTOR_HPP
#define MTGBOT__REACTOR_HPP

#include <muika/Muika.hpp>
#include <muika/OutboundMessage.hpp>
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

	using muika::Reactor::sendMsgText;
	using muika::Reactor::sendMsgPhotoUrl;
	using muika::Reactor::sendMsgPhotoFile;

	virtual std::string sendMsgText(const std::string &chat_id,
					const muika::OutboundMessage &om) override;
	virtual std::string sendMsgPhotoUrl(const std::string &chat_id,
					    const std::string &photo_url,
					    const muika::OutboundMessage &om) override;
	virtual std::string sendMsgPhotoFile(const std::string &chat_id,
					     const std::string &photo_file_path,
					     const muika::OutboundMessage &om) override;

private:
	Bot *bot_;
	TgBot::Bot *bot(void);
};

} /* namespace mtgbot */

#endif /* MTGBOT__REACTOR_HPP */
