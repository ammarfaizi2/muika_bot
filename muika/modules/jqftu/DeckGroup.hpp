// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECKGROUP_HPP
#define MUIKA__MODULES__JQFTU__DECKGROUP_HPP

#include "muika/modules/jqftu/Card.hpp"
#include "muika/modules/jqftu/Deck.hpp"

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace muika {
namespace modules {
namespace jqftu {

class DeckGroup {
private:
	std::vector<std::unique_ptr<Deck>> decks_;

public:
	using json = nlohmann::json;

	DeckGroup(void) = default;
	~DeckGroup(void) = default;
	static std::unique_ptr<Deck> createDeckByName(const std::string &name);
	static std::unique_ptr<Deck> createDeckByJson(const json &j);
	void addDeckByName(const std::string &name);
	void addDeck(std::unique_ptr<Deck> deck);
	void addDeckByJson(const json &j);
	void shuffleAllDecks(void);
	Deck *drawDeck(void);

	json toJson(void);
	void fromJson(const json &j);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECKGROUP_HPP */
