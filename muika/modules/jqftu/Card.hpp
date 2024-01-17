// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__CARD_HPP
#define MUIKA__MODULES__JQFTU__CARD_HPP

#include <string>

namespace muika {
namespace modules {
namespace jqftu {

struct Card {
protected:
	bool normalizedJapaneseCompare(const std::string &a, const std::string &b) const;

public:
	Card(void) = default;
	virtual ~Card(void) = default;
	virtual bool answer(const std::string &answer) const = 0;
	virtual std::string getAnswerInfo(void) = 0;
	virtual std::string getQuestion(void) = 0;
	virtual std::string getQuestionInfo(void) = 0;
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__CARD_HPP */
