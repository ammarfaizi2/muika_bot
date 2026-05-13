// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/deck_foundations/kotoba/Deck.hpp>

#include <stdexcept>

namespace muika {
namespace modules {
namespace m002_jqftu {
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

	kanji = j["kanji"].get<std::string>();
	kana = j["kana"].get<std::string>();
	romaji = j["romaji"].get<std::string>();
	en = j["en"].get<std::string>();
	return std::make_unique<Card>(kanji, kana, romaji, en);
}

void Deck::loadDeckFromJsonFile(const char *file_path)
{
	std::string json_str = getDeckJsonString(file_path);
	json j = json::parse(json_str);

	if (!j.is_array())
		throw std::runtime_error(std::string("Failed to parse deck: ")
					 + file_path + ": Not an array");

	if (j.empty())
		throw std::runtime_error(std::string("Failed to parse deck: ")
					 + file_path + ": Empty array");

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
		throw std::runtime_error("Deck JSON missing/invalid \"name\"");
	if (!j.contains("info") || !j["info"].is_string())
		throw std::runtime_error("Deck JSON missing/invalid \"info\"");
	if (!j.contains("scope") || !j["scope"].is_string())
		throw std::runtime_error("Deck JSON missing/invalid \"scope\"");
	if (!j.contains("cards") || !j["cards"].is_array())
		throw std::runtime_error("Deck JSON missing/invalid \"cards\"");

	if (j.contains("current_card_index") && j["current_card_index"].is_number_unsigned())
		setCurrentCardIndex(j["current_card_index"].get<uint32_t>());

	setName(j["name"].get<std::string>());
	setInfo(j["info"].get<std::string>());
	setScope(j["scope"].get<std::string>());
	for (auto &card : j["cards"])
		addCard(loadCardFromJson(card));
}

} /* namespace kotoba */
} /* namespace deck_foundations */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
