// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/decks/yamanote_line/Deck.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace decks {
namespace yamanote_line {

Deck::Deck(const std::string &decks_dir):
	muika::modules::m002_jqftu::deck_foundations::japan_eki::Deck(decks_dir)
{
	setName("yamanote_line");
	setScope("Yamanote Line");
	loadDeckFromJsonFile("yamanote_line.json");
}

} /* namespace yamanote_line */
} /* namespace decks */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
