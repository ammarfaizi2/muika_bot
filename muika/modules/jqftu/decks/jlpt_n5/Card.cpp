// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/jlpt_n5/Card.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace jlpt_n5 {

bool Card::answer(const std::string &answer) const
{
	if (answer == kana_)
		return true;

	if (normalizedJapaneseCompare(answer, romaji_))
		return true;

	return false;
}

std::string Card::getAnswerInfo(void)
{
	return  "Q: " + kanji_ + "\n"
		"English: " + en_ + "\n"
		"Answer: " + kana_ + " (" + romaji_ + ")\n";
}

std::string Card::getQuestion(void)
{
	return kanji_;
}

std::string Card::getQuestionInfo(void)
{
	return  "Scope: JLPT N5 Kotoba Deck\n\n"
		"Write the kana or romaji for the following word!";
}

} /* namespace muika::modules::jqftu::decks::jlpt_n5 */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
