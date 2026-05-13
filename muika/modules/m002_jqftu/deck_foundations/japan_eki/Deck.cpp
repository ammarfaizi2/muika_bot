// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/deck_foundations/japan_eki/Deck.hpp>

#include <stdexcept>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace deck_foundations {
namespace japan_eki {

// static
std::unique_ptr<Card> Deck::loadCardFromJson(const json &j)
{
	std::string n, kanji, romaji, hiragana, katakana, q_img;
	std::vector<std::string> alternatives;
	std::vector<std::string> photos;

	if (!j.is_object())
		throw std::runtime_error("Card JSON is not an object");

	if (!j.contains("n") || !j["n"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"n\"");
	if (!j.contains("kanji") || !j["kanji"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"kanji\"");
	if (!j.contains("romaji") || !j["romaji"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"romaji\"");
	if (!j.contains("hiragana") || !j["hiragana"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"hiragana\"");
	if (!j.contains("katakana") || !j["katakana"].is_string())
		throw std::runtime_error("Card JSON is missing or invalid \"katakana\"");

	if (j.contains("alt")) {
		if (!j["alt"].is_array())
			throw std::runtime_error("\"alt\" key is not an array");
		for (const auto &alt: j["alt"]) {
			if (!alt.is_string())
				throw std::runtime_error("Non-string in \"alt\"");
			alternatives.push_back(alt.get<std::string>());
		}
	}

	if (j.contains("q_img") && j["q_img"].is_string())
		q_img = j["q_img"].get<std::string>();

	if (j.contains("photos") && j["photos"].is_array()) {
		for (const auto &photo: j["photos"]) {
			if (!photo.is_string())
				throw std::runtime_error("Non-string in \"photos\"");
			std::string tmp = photo.get<std::string>();
			if (tmp.find("https://telegram-bot.teainside.org/output/") == 0)
				photos.push_back(tmp);
			else
				photos.push_back("https://telegram-bot.teainside.org/output/" + tmp);
		}
	}

	n = j["n"].get<std::string>();
	kanji = j["kanji"].get<std::string>();
	romaji = j["romaji"].get<std::string>();
	hiragana = j["hiragana"].get<std::string>();
	katakana = j["katakana"].get<std::string>();
	return std::make_unique<Card>(n, kanji, romaji, hiragana, katakana,
				      alternatives, q_img, photos);
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

} /* namespace japan_eki */
} /* namespace deck_foundations */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
