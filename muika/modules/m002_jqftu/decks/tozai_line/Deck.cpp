// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/decks/tozai_line/Deck.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace decks {
namespace tozai_line {

Deck::Deck(void)
{
	setName("tozai_line");
	setScope("Tokyo Metro Tozai Line");
	loadDeckFromJsonFile("tozai_line.json");
}

} /* namespace tozai_line */
} /* namespace decks */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
