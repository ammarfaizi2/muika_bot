// SPDX-License-Identifier: GPL-2.0-only

#include "Deck.hpp"
#include "internal.hpp"

#include <mutex>
#include <random>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {

Deck::Deck(std::string name):
	name_(name)
{
}

Card *Deck::drawCard(void)
{
	if (current_card_ >= cards_.size())
		return NULL;

	return &cards_[current_card_++];
}

char *Deck::getDeckFilePath(void)
{
	static const char file_base_path[] = "./storage/jqftu/decks/";
	char *file_path;
	size_t len;

	/*
	 * Don't allow '.' and '/' in the file name.
	 */
	for (auto &c : name_) {
		if (c == '.' || c == '/')
			return NULL;
	}

	len = strlen(file_base_path) + name_.length() + 6;
	file_path = (char *)malloc(len);
	if (!file_path)
		return NULL;

	snprintf(file_path, len, "%s%s.json", file_base_path, name_.c_str());
	pr_debug("Deck file path: %s\n", file_path);
	return file_path;
}

char *Deck::getDeckJsonString(void)
{
	char *json_str;
	long file_size;
	char *path;
	FILE *fp;

	path = getDeckFilePath();
	if (!path)
		return NULL;

	fp = fopen(path, "rb");
	free(path);
	if (!fp) {
		pr_debug("Failed to open deck file: %s: %s\n", strerror(errno), path);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	json_str = (char *)malloc(file_size + 1);
	if (!json_str) {
		fclose(fp);
		pr_debug("Failed to allocate memory for deck file: %s\n", path);
		return NULL;
	}

	if (fread(json_str, 1, file_size, fp) != (size_t)file_size) {
		fclose(fp);
		free(json_str);
		pr_debug("Failed to read deck file: %s\n", path);
		return NULL;
	}

	json_str[file_size] = '\0';
	return json_str;
}

int Deck::loadDeck(void)
{
	char *json_str;
	json j;

	json_str = getDeckJsonString();
	if (!json_str)
		return -1;

	try {
		j = json::parse(json_str);
	} catch (json::parse_error &e) {
		pr_debug("Failed to parse deck file: %s\n", e.what());
		free(json_str);
		return -1;
	}

	if (!j.is_array()) {
		pr_debug("Deck file is not an array.\n");
		free(json_str);
		return -1;
	}

	for (auto &card : j) {
		Card c;

		if (!card.is_object()) {
			pr_debug("Card is not an object.\n");
			free(json_str);
			return -1;
		}

		if (!card["kanji"].is_string() ||
		    !card["romaji"].is_string() ||
		    !card["hiragana"].is_string() ||
		    !card["katakana"].is_string() ||
		    !card["n"].is_string()) {
			pr_debug("Card field is not a string.\n");
			free(json_str);
			return -1;
		}

		c.kanji = card["kanji"].get<std::string>();
		c.romaji = card["romaji"].get<std::string>();
		c.hiragana = card["hiragana"].get<std::string>();
		c.katakana = card["katakana"].get<std::string>();
		c.extra = card["n"].get<std::string>();
		cards_.push_back(c);
	}

	free(json_str);
	auto rng = std::default_random_engine {};
	std::shuffle(std::begin(cards_), std::end(cards_), rng);
	return 0;
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
