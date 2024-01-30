// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/jlpt_n5/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace jlpt_n5 {

Deck::Deck(void)
{
	setName("jlpt_n5");
	setScope("JLPT N5 Kotoba Deck");
	loadDeckFromJsonFile("jlpt_n5.json");
}

} /* namespace muika::modules::jqftu::decks::jlpt_n5 */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
