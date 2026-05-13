// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__DECKS__TOZAI_LINE__DECK_HPP
#define MUIKA__MODULES__M002_JQFTU__DECKS__TOZAI_LINE__DECK_HPP

#include <muika/modules/m002_jqftu/deck_foundations/japan_eki/Deck.hpp>

#include <string>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace decks {
namespace tozai_line {

class Deck: public muika::modules::m002_jqftu::deck_foundations::japan_eki::Deck {
public:
	explicit Deck(const std::string &decks_dir);
};

} /* namespace tozai_line */
} /* namespace decks */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__M002_JQFTU__DECKS__TOZAI_LINE__DECK_HPP */
