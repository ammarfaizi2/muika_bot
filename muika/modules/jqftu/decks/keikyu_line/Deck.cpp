// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/keikyu_line/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace keikyu_line {

Deck::Deck(void)
{
	setName("keikyu_line");
	setScope("Keikyu Line");
	loadDeckFromJsonFile("keikyu_line.json");
}

} /* namespace muika::modules::jqftu::decks::keikyu_line */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
