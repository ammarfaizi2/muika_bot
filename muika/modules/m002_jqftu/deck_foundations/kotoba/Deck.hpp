// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__DECK_FOUNDATIONS__KOTOBA__DECK_HPP
#define MUIKA__MODULES__M002_JQFTU__DECK_FOUNDATIONS__KOTOBA__DECK_HPP

#include <muika/modules/m002_jqftu/Deck.hpp>
#include <muika/modules/m002_jqftu/deck_foundations/kotoba/Card.hpp>

#include <memory>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace deck_foundations {
namespace kotoba {

class Deck: public muika::modules::m002_jqftu::Deck {
private:
	static std::unique_ptr<Card> loadCardFromJson(const json &j);

protected:
	void loadDeckFromJsonFile(const char *file_path);

public:
	Deck(void) = default;

	virtual json toJson(void) const override;
	virtual void fromJson(const json &j) override;
};

} /* namespace kotoba */
} /* namespace deck_foundations */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__M002_JQFTU__DECK_FOUNDATIONS__KOTOBA__DECK_HPP */
