// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Deck.hpp"
#include "muika/modules/jqftu/decks/tozai_line/Deck.hpp"

#include <stdexcept>

namespace muika {
namespace modules {
namespace jqftu {

std::unique_ptr<Deck> Deck::createDeck(const std::string &name)
{
	if (name == "tozai_line")
		return std::make_unique<decks::tozai_line::Deck>();

	throw std::invalid_argument("Unknown deck name: " + name);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
