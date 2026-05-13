// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__DECKGROUP_HPP
#define MUIKA__MODULES__M002_JQFTU__DECKGROUP_HPP

#include <muika/modules/m002_jqftu/Deck.hpp>

#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {

class DeckGroup {
private:
	std::string decks_dir_;
	std::vector<std::unique_ptr<Deck>> decks_;

public:
	using json = ::nlohmann::json;

	explicit DeckGroup(std::string decks_dir);
	~DeckGroup(void) = default;

	std::unique_ptr<Deck> createDeckByName(const std::string &name) const;
	std::unique_ptr<Deck> createDeckByJson(const json &j) const;

	void addDeckByName(const std::string &name);
	void addDeck(std::unique_ptr<Deck> deck);
	void addDeckByJson(const json &j);
	void shuffleAllDecks(void);
	Deck *drawDeck(void);

	json toJson(void) const;
	void fromJson(const json &j);
};

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__M002_JQFTU__DECKGROUP_HPP */
