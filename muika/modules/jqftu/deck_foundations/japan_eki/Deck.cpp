// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/deck_foundations/japan_eki/Card.hpp"
#include "muika/modules/jqftu/deck_foundations/japan_eki/Deck.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace deck_foundations {
namespace japan_eki {

// static
std::unique_ptr<Card> Deck::loadCardFromJson(const json &j)
{
	std::string n, kanji, romaji, hiragana, katakana, q_img = "";
	std::vector<std::string> alternatives = {};
	std::vector<std::string> photos = {};

	if (!j.is_object())
		throw std::runtime_error("Card JSON is not an object");

	if (!j.contains("n") || !j["n"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"n\" field");

	if (!j.contains("kanji") || !j["kanji"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"kanji\" field");

	if (!j.contains("romaji") || !j["romaji"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"romaji\" field");

	if (!j.contains("hiragana") || !j["hiragana"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"hiragana\" field");

	if (!j.contains("katakana") || !j["katakana"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"katakana\" field");

	if (j.contains("alt")) {
		if (!j["alt"].is_array())
			throw std::runtime_error("\"alt\" key is not an array");

		for (const auto &alt: j["alt"]) {
			if (!alt.is_string())
				throw std::runtime_error("Non-string value in \"alt\" array");
		}

		for (const auto &alt: j["alt"])
			alternatives.push_back(alt);
	}

	if (j.contains("q_img") && j["q_img"].is_string()) {
		q_img = j["q_img"];
	}

	if (j.contains("photos") && j["photos"].is_array()) {
		for (const auto &photo: j["photos"]) {
			if (!photo.is_string())
				throw std::runtime_error("Non-string value in \"photos\" array");

			std::string tmp = photo.get<std::string>();
			if (tmp.find("https://telegram-bot.teainside.org/output/") == 0)
				photos.push_back(tmp);
			else
				photos.push_back("https://telegram-bot.teainside.org/output/" + tmp);
		}
	}

	n = j["n"];
	kanji = j["kanji"];
	romaji = j["romaji"];
	hiragana = j["hiragana"];
	katakana = j["katakana"];
	return std::make_unique<Card>(n, kanji, romaji, hiragana, katakana, alternatives, q_img, photos);
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

} /* namespace muika::modules::jqftu::decks::ja_train */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
