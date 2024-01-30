// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECK_FOUNDATION__KOTOBA__CARD_HPP
#define MUIKA__MODULES__JQFTU__DECK_FOUNDATION__KOTOBA__CARD_HPP

#include "muika/modules/jqftu/Card.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace deck_foundations {
namespace kotoba {

class Card: public muika::modules::jqftu::Card {
private:
	std::string kanji_;
	std::string kana_;
	std::string romaji_;
	std::string en_;

public:
	inline Card(const std::string &kanji, const std::string &kana,
		    const std::string &romaji, const std::string &en):
		kanji_(kanji),
		kana_(kana),
		romaji_(romaji),
		en_(en)
	{
	}

	inline Card(void) = default;

	virtual bool checkAnswer(const std::string &answer) override;
	virtual std::string getCardImage(void) override;
	virtual std::string getCardCaption(void) override;
	virtual std::string getCardAnswer(void) override;
	virtual std::string getCardDetails(void) override;
	virtual json toJson(void) const override;
	virtual void fromJson(const json &j) override;
};

} /* namespace muika::modules::jqftu::deck_foundations::kotoba */
} /* namespace muika::modules::jqftu::deck_foundations */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECK_FOUNDATION__KOTOBA__CARD_HPP */
