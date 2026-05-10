// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECK_FOUNDATION__JAPAN_EKI__CARD_HPP
#define MUIKA__MODULES__JQFTU__DECK_FOUNDATION__JAPAN_EKI__CARD_HPP

#include "muika/modules/jqftu/Card.hpp"

#include <mutex>
#include <string>

namespace muika {
namespace modules {
namespace jqftu {
namespace deck_foundations {
namespace japan_eki {

class Card: public muika::modules::jqftu::Card {
private:
	std::string n_;
	std::string kanji_;
	std::string romaji_;
	std::string hiragana_;
	std::string katakana_;

public:
	inline Card(const std::string &n, const std::string &kanji,
		    const std::string &romaji, const std::string &hiragana,
		    const std::string &katakana):
		n_(n),
		kanji_(kanji),
		romaji_(romaji),
		hiragana_(hiragana),
		katakana_(katakana)
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

} /* namespace muika::modules::jqftu::deck_foundations::japan_eki */
} /* namespace muika::modules::jqftu::deck_foundations */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECK_FOUNDATION__JAPAN_EKI__CARD_HPP */
