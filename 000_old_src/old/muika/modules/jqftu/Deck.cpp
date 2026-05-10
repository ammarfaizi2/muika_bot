// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Deck.hpp"
#include "muika/modules/jqftu/internal.hpp"

#include <mutex>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>

using json = nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {

std::string Deck::getDeckJsonString(const char *file_name)
{
	char path[4096];
	size_t len;
	FILE *fp;

	snprintf(path, sizeof(path), "./storage/jqftu/decks/%s", file_name);
	fp = fopen(path, "rb");
	if (!fp)
		throw std::runtime_error("Failed to open deck file: " + std::string(strerror(errno)) + ": " + path);

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (len == 0) {
		fclose(fp);
		throw std::runtime_error("Deck file is empty: " + std::string(path));
	}

	try {
		std::string str(len, '\0');
		size_t tmp;

		tmp = fread(&str[0], 1, len, fp);
		fclose(fp);
		if (tmp != len)
			throw std::runtime_error("Failed to read deck file: " + std::string(path));

		return str;
	} catch (const std::bad_alloc &e) {
		fclose(fp);
		throw std::runtime_error("Failed to allocate memory for deck file: " + std::string(e.what()));
	}
}

Card *Deck::drawCard(void)
{
	if (current_card_index_ >= cards_.size())
		return nullptr;

	return cards_[current_card_index_++].get();
}

void Deck::shuffle(void)
{
	std::random_shuffle(cards_.begin(), cards_.end());
	current_card_index_ = 0;
}

void Deck::clearCards(void)
{
	cards_.clear();
	current_card_index_ = 0;
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
