// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/deck_foundations/kotoba/Card.hpp"
#include "muika/modules/jqftu/deck_foundations/kotoba/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace deck_foundations {
namespace kotoba {

// static
std::unique_ptr<Card> Deck::loadCardFromJson(const json &j)
{
	std::string kanji, kana, romaji, en;

	if (!j.is_object())
		throw std::runtime_error("Card JSON is not an object");

	if (!j.contains("kanji") || !j["kanji"].is_string())
		throw std::runtime_error("No string \"kanji\" key in JSON");

	if (!j.contains("kana") || !j["kana"].is_string())
		throw std::runtime_error("No string \"kana\" key in JSON");

	if (!j.contains("romaji") || !j["romaji"].is_string())
		throw std::runtime_error("No string \"romaji\" key in JSON");

	if (!j.contains("en") || !j["en"].is_string())
		throw std::runtime_error("No string \"en\" key in JSON");

	kanji = j["kanji"];
	kana = j["kana"];
	romaji = j["romaji"];
	en = j["en"];
	return std::make_unique<Card>(kanji, kana, romaji, en);
}

void Deck::loadDeckFromJsonFile(const char *file_path)
{
	std::string json_str;
	json j;

	json_str = getDeckJsonString(file_path);
	j = json::parse(json_str);
	if (!j.is_array())
		throw std::runtime_error("Failed to parse deck: " + std::string(file_path) + ": Not an array");

	if (j.size() == 0)
		throw std::runtime_error("Failed to parse deck: " + std::string(file_path) + ": Empty array");

	for (auto &card : j)
		addCard(loadCardFromJson(card));
}

json Deck::toJson(void) const
{
	uint32_t cur_idx = getCurrentCardIndex();
	json j;

	if (cur_idx > 0)
		cur_idx--;

	j["name"] = getName();
	j["info"] = getInfo();
	j["scope"] = getScope();
	j["cards"] = json::array();
	j["current_card_index"] = cur_idx;
	auto &cards = getCards();
	for (auto &card : cards)
		j["cards"].push_back(card->toJson());

	return j;
}

void Deck::fromJson(const json &j)
{
	if (!j.is_object())
		throw std::runtime_error("Deck JSON is not an object");

	if (!j.contains("name") || !j["name"].is_string())
		throw std::runtime_error("Deck JSON is missing or invalid \"name\" field");

	if (!j.contains("info") || !j["info"].is_string())
		throw std::runtime_error("Deck JSON is missing or invalid \"info\" field");

	if (!j.contains("scope") || !j["scope"].is_string())
		throw std::runtime_error("Deck JSON is missing or invalid \"scope\" field");

	if (!j.contains("cards") || !j["cards"].is_array())
		throw std::runtime_error("Deck JSON is missing or invalid \"cards\" field");

	if (j.contains("current_card_index") && j["current_card_index"].is_number_unsigned())
		setCurrentCardIndex(j["current_card_index"]);

	setName(j["name"]);
	setInfo(j["info"]);
	setScope(j["scope"]);
	for (auto &card : j["cards"])
		addCard(loadCardFromJson(card));
}

} /* namespace muika::modules::jqftu::decks::kotoba */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
