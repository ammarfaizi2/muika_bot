// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/tozai_line/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace tozai_line {

Deck::Deck(void)
{
	setName("tozai_line");
	setScope("Tokyo Metro Tozai Line");
	loadDeckFromJsonFile("tozai_line.json");
}

} /* namespace muika::modules::jqftu::decks::tozai_line */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
