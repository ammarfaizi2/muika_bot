// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/decks/chuo_sobu/Deck.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace decks {
namespace chuo_sobu {

Deck::Deck(void)
{
	setName("chuo_sobu");
	setScope("Chuo Sobu Line");
	loadDeckFromJsonFile("chuo_sobu.json");
}

} /* namespace chuo_sobu */
} /* namespace decks */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
