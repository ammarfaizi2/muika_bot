// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/tozai_line/Card.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace tozai_line {

bool Card::answer(const std::string &answer) const
{
	if (answer == hiragana_ || answer == katakana_)
		return true;

	if (normalizedJapaneseCompare(answer, romaji_))
		return true;

	return false;
}

std::string Card::getAnswerInfo(void)
{
	return  "Q: " + kanji_ + "\n"
		"Station number: " + n_ + "\n"
		"Answer: " + hiragana_ + " (" + romaji_ + ")\n";
}

std::string Card::getQuestion(void)
{
	return kanji_;
}

std::string Card::getQuestionInfo(void)
{
	return  "Scope: Tokyo Metro Tozai Line\n\n"
		"Write the kana or romaji for the following station name!";
}

} /* namespace muika::modules::jqftu::decks::tozai_line */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
