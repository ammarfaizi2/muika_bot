// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Deck.hpp>
#include <muika/helpers.hpp>

#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace muika {
namespace modules {
namespace m002_jqftu {

Deck::Deck(std::string decks_dir):
	decks_dir_(std::move(decks_dir))
{
}

std::string Deck::getDeckJsonString(const char *file_name) const
{
	std::string path = decks_dir_ + "/" + file_name;

	std::string contents;
	int r = mk_file_get_contents(path, contents);
	if (r < 0)
		throw std::runtime_error("Failed to read deck file: "
					 + std::string(strerror(-r)) + ": " + path);

	if (contents.empty())
		throw std::runtime_error("Deck file is empty: " + path);

	return contents;
}

Card *Deck::drawCard(void)
{
	if (current_card_index_ >= cards_.size())
		return nullptr;

	return cards_[current_card_index_++].get();
}

void Deck::shuffle(void)
{
	thread_local std::mt19937 rng{std::random_device{}() ^ static_cast<uint32_t>(
		std::chrono::high_resolution_clock::now().time_since_epoch().count())};
	std::shuffle(cards_.begin(), cards_.end(), rng);
	current_card_index_ = 0;
}

void Deck::clearCards(void)
{
	cards_.clear();
	current_card_index_ = 0;
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
