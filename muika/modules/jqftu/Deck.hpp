// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__DECK_HPP
#define MUIKA__DECK_HPP

#include "Card.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace muika {
namespace modules {
namespace jqftu {

class Deck {
private:
	std::string name_;
	std::vector<Card> cards_;
	uint32_t current_card_ = 0;
	char *getDeckFilePath(void);
	char *getDeckJsonString(void);
public:
	Deck(std::string name);
	~Deck(void) = default;
	int loadDeck(void);
	Card *drawCard(void);
	inline bool isFinished(void) const { return current_card_ >= cards_.size(); }
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__DECK_HPP */
