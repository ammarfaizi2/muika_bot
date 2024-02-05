// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/yamanote_line/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace yamanote_line {

Deck::Deck(void)
{
	setName("yamanote_line");
	setScope("Yamanote Line");
	loadDeckFromJsonFile("yamanote_line.json");
}

} /* namespace muika::modules::jqftu::decks::yamanote_line */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
