// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECKS__TOZAI_LINE__CARD_HPP
#define MUIKA__MODULES__JQFTU__DECKS__TOZAI_LINE__CARD_HPP

#include "muika/modules/jqftu/Card.hpp"

#include <string>

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace tozai_line {

class Card: public muika::modules::jqftu::Card {
private:
	std::string n_;
	std::string kanji_;
	std::string romaji_;
	std::string hiragana_;
	std::string katakana_;

public:
	Card(void) = default;

	inline Card(
		const std::string &n,
		const std::string &kanji,
		const std::string &romaji,
		const std::string &hiragana,
		const std::string &katakana
	):
		n_(n),
		kanji_(kanji),
		romaji_(romaji),
		hiragana_(hiragana),
		katakana_(katakana)
	{
	}

	virtual ~Card(void) = default;
	virtual bool answer(const std::string &answer) const override;
	virtual std::string getAnswerInfo(void) override;
	virtual std::string getQuestion(void) override;
	virtual std::string getQuestionInfo(void) override;
};

} /* namespace muika::modules::jqftu::decks::tozai_line */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECKS__TOZAI_LINE__CARD_HPP */
