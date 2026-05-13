// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__DECK_FOUNDATIONS__JAPAN_EKI__CARD_HPP
#define MUIKA__MODULES__M002_JQFTU__DECK_FOUNDATIONS__JAPAN_EKI__CARD_HPP

#include <muika/modules/m002_jqftu/Card.hpp>

#include <string>
#include <vector>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace deck_foundations {
namespace japan_eki {

class Card: public muika::modules::m002_jqftu::Card {
private:
	std::string n_;
	std::string kanji_;
	std::string romaji_;
	std::string hiragana_;
	std::string katakana_;
	std::vector<std::string> alt_;

	std::string q_img_;
	std::vector<std::string> photos_;

public:
	inline Card(const std::string &n, const std::string &kanji,
		    const std::string &romaji, const std::string &hiragana,
		    const std::string &katakana,
		    std::vector<std::string> alt = {},
		    const std::string &q_img = "",
		    std::vector<std::string> photos = {}):
		n_(n),
		kanji_(kanji),
		romaji_(romaji),
		hiragana_(hiragana),
		katakana_(katakana),
		alt_(std::move(alt)),
		q_img_(q_img),
		photos_(std::move(photos))
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
	virtual const std::vector<std::string> &getCardPhotos(void) override;
};

} /* namespace japan_eki */
} /* namespace deck_foundations */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__M002_JQFTU__DECK_FOUNDATIONS__JAPAN_EKI__CARD_HPP */
