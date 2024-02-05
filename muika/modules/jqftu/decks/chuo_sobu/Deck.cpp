// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/chuo_sobu/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace chuo_sobu {

Deck::Deck(void)
{
	setName("chuo_sobu");
	setScope("Chuo Sobu Line");
	loadDeckFromJsonFile("chuo_sobu.json");
}

} /* namespace muika::modules::jqftu::decks::chuo_sobu */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
