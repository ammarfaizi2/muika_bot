// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/DeckGroup.hpp>

#include <chrono>
#include <random>
#include <stdexcept>

namespace muika {
namespace modules {
namespace m002_jqftu {

// static
std::unique_ptr<Deck> DeckGroup::createDeckByName(const std::string &name)
{
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
	std::vector<Deck *> unfinished;
	for (auto &deck : decks_) {
		if (deck->getNumberOfCardsLeft() > 0)
			unfinished.push_back(deck.get());
	}

	if (unfinished.empty())
		return nullptr;

	thread_local std::mt19937 rng{std::random_device{}() ^ static_cast<uint32_t>(
		std::chrono::high_resolution_clock::now().time_since_epoch().count())};
	std::uniform_int_distribution<size_t> dist(0, unfinished.size() - 1);
	return unfinished[dist(rng)];
}

DeckGroup::json DeckGroup::toJson(void) const
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

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
