// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECKS__jlpt_n5__CARD_HPP
#define MUIKA__MODULES__JQFTU__DECKS__jlpt_n5__CARD_HPP

#include "muika/modules/jqftu/Card.hpp"

#include <string>

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace jlpt_n5 {

class Card: public muika::modules::jqftu::Card {
private:
	std::string en_;
	std::string kanji_;
	std::string romaji_;
	std::string kana_;

public:
	Card(void) = default;

	inline Card(
		const std::string &en,
		const std::string &kanji,
		const std::string &romaji,
		const std::string &kana
	):
		en_(en),
		kanji_(kanji),
		romaji_(romaji),
		kana_(kana)
	{
	}

	virtual ~Card(void) = default;
	virtual bool answer(const std::string &answer) const override;
	virtual std::string getAnswerInfo(void) override;
	virtual std::string getQuestion(void) override;
	virtual std::string getQuestionInfo(void) override;
};

} /* namespace muika::modules::jqftu::decks::jlpt_n5 */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECKS__jlpt_n5__CARD_HPP */
