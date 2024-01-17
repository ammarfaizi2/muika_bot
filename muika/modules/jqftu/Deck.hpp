// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECK_HPP
#define MUIKA__MODULES__JQFTU__DECK_HPP

#include "muika/modules/jqftu/Card.hpp"

#include <string>
#include <memory>

namespace muika {
namespace modules {
namespace jqftu {

class Deck {
protected:
	std::string name_;
	inline void setName(const std::string &name) { name_ = name; }

public:
	Deck(void) = default;
	virtual ~Deck(void) = default;
	virtual void shuffle(void) = 0;
	virtual Card *draw(void) = 0;
	virtual bool isFinished(void) const = 0;

	const std::string &getName(void) const { return name_; }
	static std::unique_ptr<Deck> createDeck(const std::string &name);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECK_HPP */
