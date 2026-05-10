// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/DeckGroup.hpp"
#include "muika/modules/jqftu/decks/tozai_line/Deck.hpp"

#include <mutex>
#include <string>
#include <vector>
#include <cstdlib>

using json = nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {

// static
std::unique_ptr<Deck> DeckGroup::createDeckByName(const std::string &name)
{
	if (name == "tozai_line")
		return std::make_unique<muika::modules::jqftu::decks::tozai_line::Deck>();

	throw std::runtime_error("Unknown deck: " + name);
}

// static
std::unique_ptr<Deck> DeckGroup::createDeckByJson(const json &j)
{
	if (!j.contains("name"))
		throw std::runtime_error("No \"name\" key in JSON");

	if (!j["name"].is_string())
		throw std::runtime_error("JSON \"name\" key is not a string");

	std::unique_ptr<Deck> deck = createDeckByName(j["name"].get<std::string>());
	deck->clearCards();
	deck->fromJson(j);
	return deck;
}

void DeckGroup::addDeck(std::unique_ptr<Deck> deck)
{
	decks_.push_back(std::move(deck));
}

void DeckGroup::addDeckByName(const std::string &name)
{
	addDeck(createDeckByName(name));
}

void DeckGroup::addDeckByJson(const json &j)
{
	addDeck(createDeckByJson(j));
}

void DeckGroup::shuffleAllDecks(void)
{
	for (auto &deck : decks_)
		deck->shuffle();
}

Deck *DeckGroup::drawDeck(void)
{
	std::vector<Deck *> unfinished_decks;

	for (auto &deck : decks_) {
		if (deck->getNumberOfCardsLeft() > 0)
			unfinished_decks.push_back(deck.get());
	}

	if (unfinished_decks.size() == 0)
		return nullptr;

	return unfinished_decks[rand() % unfinished_decks.size()];
}

json DeckGroup::toJson(void)
{
	json j;

	j["decks"] = json::array();
	for (auto &deck : decks_)
		j["decks"].push_back(deck->toJson());

	return j;
}

void DeckGroup::fromJson(const json &j)
{
	if (!j.contains("decks"))
		throw std::runtime_error("No \"decks\" key in JSON");

	if (!j["decks"].is_array())
		throw std::runtime_error("JSON \"decks\" key is not an array");

	decks_.clear();
	for (auto &deck : j["decks"])
		addDeckByJson(deck);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
