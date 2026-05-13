// SPDX-License-Identifier: GPL-2.0-only
#include <mtgbot/Reactor.hpp>
#include <muika/OutboundMessage.hpp>
#include <tgbot/tgbot.h>
#include <cstring>
#include <cstdio>

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

static std::shared_ptr<TgBot::ReplyParameters>
buildReplyParams(const std::string &chat_id, const std::string &reply_to)
{
	if (reply_to.empty())
		return nullptr;

	auto rep = std::make_shared<TgBot::ReplyParameters>();
	initReplyParameters(rep.get());
	try {
		rep->messageId = std::stoi(reply_to);
		rep->chatId = std::stoll(chat_id);
	} catch (...) {
		return nullptr;
	}
	rep->allowSendingWithoutReply = true;
	return rep;
}

static TgBot::GenericReply::Ptr
buildKeyboard(const muika::InlineKeyboard &kb)
{
	if (kb.empty())
		return nullptr;

	auto markup = std::make_shared<TgBot::InlineKeyboardMarkup>();
	for (const auto &row : kb) {
		std::vector<TgBot::InlineKeyboardButton::Ptr> out_row;
		for (const auto &btn : row) {
			auto b = std::make_shared<TgBot::InlineKeyboardButton>();
			b->text = btn.text;
			b->callbackData = btn.callback_data;
			b->url = btn.url;
			out_row.push_back(b);
		}
		markup->inlineKeyboard.push_back(out_row);
	}
	return markup;
}

static std::string idToStr(std::int32_t id)
{
	return std::to_string(id);
}

std::string Reactor::sendMsgText(const std::string &chat_id,
				 const muika::OutboundMessage &om)
{
	auto rep = buildReplyParams(chat_id, om.reply_to_msg_id);
	auto kb = buildKeyboard(om.inline_keyboard);

	printf("Reactor::sendMsgText: chat_id=%s, text=%s, reply_to_msg_id=%s\n",
	       chat_id.c_str(), om.text.c_str(), om.reply_to_msg_id.c_str());

	try {
		auto m = bot()->getApi().sendMessage(chat_id, om.text,
						     nullptr, rep, kb,
						     om.parse_mode,
						     om.disable_web_page_preview);
		if (m)
			return idToStr(m->messageId);
	} catch (const std::exception &e) {
		printf("Reactor::sendMsgText: error: %s\n", e.what());
	}
	return "";
}

std::string Reactor::sendMsgPhotoUrl(const std::string &chat_id,
				     const std::string &photo_url,
				     const muika::OutboundMessage &om)
{
	auto rep = buildReplyParams(chat_id, om.reply_to_msg_id);
	auto kb = buildKeyboard(om.inline_keyboard);

	try {
		auto m = bot()->getApi().sendPhoto(chat_id, photo_url, om.text,
						   rep, kb, om.parse_mode);
		if (m)
			return idToStr(m->messageId);
	} catch (const std::exception &e) {
		printf("Reactor::sendMsgPhotoUrl: error: %s\n", e.what());
	}
	return "";
}

std::string Reactor::sendMsgPhotoFile(const std::string &chat_id,
				      const std::string &photo_file_path,
				      const muika::OutboundMessage &om)
{
	auto rep = buildReplyParams(chat_id, om.reply_to_msg_id);
	auto kb = buildKeyboard(om.inline_keyboard);

	try {
		auto m = bot()->getApi().sendPhoto(chat_id,
			TgBot::InputFile::fromFile(photo_file_path, "image/jpeg"),
			om.text, rep, kb, om.parse_mode);
		if (m)
			return idToStr(m->messageId);
	} catch (const std::exception &e) {
		printf("Reactor::sendMsgPhotoFile: error: %s\n", e.what());
	}
	return "";
}

} /* namespace mtgbot */
