
#include "muika/modules/jqftu/decks/jlpt_n5/Deck.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <stdexcept>
#include <random>

using json = nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace jlpt_n5 {

Deck::Deck(void)
{
	setName("jlpt_n5");
	loadDeck();
}

inline char *Deck::getDeckJsonString(void)
{
	size_t file_size;
	char *ret;
	FILE *fp;

	fp = fopen("./storage/jqftu/decks/jlpt_n5.json", "rb");
	if (!fp)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	file_size = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ret = (char *)malloc(file_size + 1);
	if (!ret) {
		fclose(fp);
		return nullptr;
	}

	if (fread(ret, 1, file_size, fp) != (size_t)file_size) {
		fclose(fp);
		free(ret);
		return nullptr;
	}

	fclose(fp);
	ret[file_size] = '\0';
	return ret;
}

inline void Deck::loadDeck(void)
{
	char *json_str;
	json j;

	json_str = getDeckJsonString();
	if (!json_str)
		throw std::runtime_error("Failed to load deck");

	try {
		j = json::parse(json_str);
	} catch (...) {
		free(json_str);
		throw std::runtime_error("Failed to parse JSON");
	}

	if (!j.is_array()) {
		free(json_str);
		throw std::runtime_error("JSON is not an array");
	}

	for (auto &card : j) {
		if (!card.is_object()) {
			free(json_str);
			throw std::runtime_error("JSON element is not an object");
		}

		if (!card.contains("en") || !card["en"].is_string()) {
			free(json_str);
			throw std::runtime_error("JSON element does not contain \"n\" or is not a string");
		}

		if (!card.contains("kanji") || !card["kanji"].is_string()) {
			free(json_str);
			throw std::runtime_error("JSON element does not contain \"kanji\" or is not a string");
		}

		if (!card.contains("romaji") || !card["romaji"].is_string()) {
			free(json_str);
			throw std::runtime_error("JSON element does not contain \"romaji\" or is not a string");
		}

		if (!card.contains("kana") || !card["kana"].is_string()) {
			free(json_str);
			throw std::runtime_error("JSON element does not contain \"hiragana\" or is not a string");
		}

		cards_.emplace_back(
			card["en"].get<std::string>(),
			card["kanji"].get<std::string>(),
			card["romaji"].get<std::string>(),
			card["kana"].get<std::string>()
		);
	}

	free(json_str);
}

void Deck::shuffle(void)
{
	std::random_device rd;
	std::mt19937 g(rd());

	std::shuffle(cards_.begin(), cards_.end(), g);
	current_card_ = 0;
}

Card *Deck::draw(void)
{
	if (current_card_ >= cards_.size())
		return nullptr;

	return &cards_[current_card_++];
}

bool Deck::isFinished(void) const
{
	return current_card_ >= cards_.size();
}

} /* namespace muika::modules::jqftu::decks::jlpt_n5 */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
