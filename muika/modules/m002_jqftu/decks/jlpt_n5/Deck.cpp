// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/decks/jlpt_n5/Deck.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace decks {
namespace jlpt_n5 {

Deck::Deck(void)
{
	setName("jlpt_n5");
	setScope("JLPT N5 Kotoba Deck");
	loadDeckFromJsonFile("jlpt_n5.json");
}

} /* namespace jlpt_n5 */
} /* namespace decks */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
