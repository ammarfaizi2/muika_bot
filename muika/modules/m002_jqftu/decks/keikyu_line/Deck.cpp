// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/decks/keikyu_line/Deck.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace decks {
namespace keikyu_line {

Deck::Deck(const std::string &decks_dir):
	muika::modules::m002_jqftu::deck_foundations::japan_eki::Deck(decks_dir)
{
	setName("keikyu_line");
	setScope("Keikyu Line");
	loadDeckFromJsonFile("keikyu_line.json");
}

} /* namespace keikyu_line */
} /* namespace decks */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
