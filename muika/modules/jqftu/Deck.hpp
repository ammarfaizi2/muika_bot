// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECK_HPP
#define MUIKA__MODULES__JQFTU__DECK_HPP

#include "muika/modules/jqftu/Card.hpp"
#include "muika/modules/jqftu/internal.hpp"

#include <mutex>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {

class Deck {
private:
	std::string name_;
	std::string info_;
	std::string scope_;
	std::vector<std::unique_ptr<Card>> cards_;
	uint32_t current_card_index_ = 0;

protected:
	inline void setName(const std::string &name) { name_ = name; }
	inline void setInfo(const std::string &info) { info_ = info; }
	inline void setScope(const std::string &scope) { scope_ = scope; }
	inline void addCard(std::unique_ptr<Card> &&card) { cards_.push_back(std::move(card)); }
	inline void setCurrentCardIndex(uint32_t index) { current_card_index_ = index; }
	inline const std::vector<std::unique_ptr<Card>> &getCards(void) const { return cards_; }
	static std::string getDeckJsonString(const char *file_name);

public:
	Deck(void) = default;
	virtual ~Deck(void) = default;
	const std::string &getName(void) const { return name_; }
	const std::string &getInfo(void) const { return name_; }
	const std::string &getScope(void) const { return scope_; }
	uint32_t getNumberOfCards(void) const { return cards_.size(); }
	uint32_t getNumberOfCardsLeft(void) const { return cards_.size() - current_card_index_; }
	uint32_t getCurrentCardIndex(void) const { return current_card_index_; }
	void clearCards(void);
	virtual Card *drawCard(void);
	virtual void shuffle(void);

	virtual json toJson(void) const = 0;
	virtual void fromJson(const json &j) = 0;
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECK_HPP */
