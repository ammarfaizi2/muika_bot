// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECK_HPP
#define MUIKA__MODULES__JQFTU__DECK_HPP

#include "Card.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace muika {
namespace modules {
namespace jqftu {

class Deck {
protected:
	std::string name_;

	inline void setName(const std::string &name) { name_ = name; }

public:
	Deck(void);
	virtual ~Deck(void) = default;
	virtual Card *drawCard(void) = 0;
	virtual void shuffle(void) = 0;
	virtual uint32_t getCardCount(void) const = 0;
	virtual uint32_t getCurrentIndex(void) const = 0;
	inline const std::string &getName(void) const { return name_; }
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECK_HPP */
