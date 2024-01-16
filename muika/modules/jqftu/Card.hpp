// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__CARD_HPP
#define MUIKA__CARD_HPP

#include <string>

namespace muika {
namespace modules {
namespace jqftu {

struct Card {
	std::string kanji;
	std::string romaji;
	std::string hiragana;
	std::string katakana;
	std::string extra;
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__CARD_HPP */
