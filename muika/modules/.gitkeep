// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__JQFTU_HPP
#define MUIKA__JQFTU_HPP

#include <tgbot/tgbot.h>
#include "Muika.hpp"
#include <vector>

namespace muika {

class Jqftu {
private:
	Muika &m_;
	TgBot::Message::Ptr &msg_;
	std::vector<std::string> args_;

	void parseArgs(void);
	void showHelp(void);
	void startJqftu(void);
	char *getDeckJsonFilePath(void);
	char *getDeckJsonString(void);
	int randomPickFromDeck(std::string &kanji, std::string &romaji,
			       std::string &hiragana, std::string &katakana,
			       std::string &extra);
public:
	Jqftu(Muika &m, TgBot::Message::Ptr &msg);
	~Jqftu(void);
	int execute(void);
};

} /* namespace muika */

#endif /* MUIKA__JQFTU_HPP */
