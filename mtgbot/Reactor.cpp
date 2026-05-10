// SPDX-License-Identifier: GPL-2.0-only
#include <mtgbot/Reactor.hpp>
#include <tgbot/tgbot.h>
#include <cstring>

namespace mtgbot {

Reactor::Reactor(Bot *bot):
	bot_(bot)
{
}

Reactor::~Reactor(void) = default;

inline TgBot::Bot *Reactor::bot(void)
{
	return bot_->bot();
}

static void initReplyParameters(TgBot::ReplyParameters *r)
{
	r->messageId = 0;
	r->chatId = 0;
	r->allowSendingWithoutReply = false;
	r->quote = "";
	r->quoteParseMode = "";
	r->quoteEntities.clear();
	r->quotePosition = 0;
}

void Reactor::sendMsgText(const std::string &chat_id,
			  const std::string &text,
			  const std::string &reply_to_msg_id)
{
	std::shared_ptr<TgBot::ReplyParameters> rep = nullptr;

	printf("Reactor::sendMsgText: chat_id=%s, text=%s, reply_to_msg_id=%s\n",
	       chat_id.c_str(), text.c_str(), reply_to_msg_id.c_str());
	if (!reply_to_msg_id.empty()) {
		rep = std::make_shared<TgBot::ReplyParameters>();
		initReplyParameters(rep.get());
		rep->messageId = std::stoi(reply_to_msg_id);
		rep->chatId = std::stoll(chat_id);
		rep->allowSendingWithoutReply = true;
	}

	bot()->getApi().sendMessage(chat_id, text, nullptr, rep);
}

void Reactor::sendMsgPhotoUrl(const std::string &chat_id,
			      const std::string &photo_url,
			      const std::string &caption,
			      const std::string &reply_to_msg_id)
{
	std::shared_ptr<TgBot::ReplyParameters> rep = nullptr;
	if (!reply_to_msg_id.empty()) {
		rep = std::make_shared<TgBot::ReplyParameters>();
		initReplyParameters(rep.get());
		rep->messageId = std::stoi(reply_to_msg_id);
		rep->chatId = std::stoll(chat_id);
		rep->allowSendingWithoutReply = true;
	}

	bot()->getApi().sendPhoto(chat_id, photo_url, caption, rep);
}

void Reactor::sendMsgPhotoFile(const std::string &chat_id,
			       const std::string &photo_file_path,
			       const std::string &caption,
			       const std::string &reply_to_msg_id)
{
	std::shared_ptr<TgBot::ReplyParameters> rep = nullptr;
	if (!reply_to_msg_id.empty()) {
		rep = std::make_shared<TgBot::ReplyParameters>();
		initReplyParameters(rep.get());
		rep->messageId = std::stoi(reply_to_msg_id);
		rep->chatId = std::stoll(chat_id);
		rep->allowSendingWithoutReply = true;
	}

	bot()->getApi().sendPhoto(chat_id,
		TgBot::InputFile::fromFile(photo_file_path, "image/jpeg"),
		caption, rep);
}

} /* namespace mtgbot */
