// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__DECK_FOUNDATION__JAPAN_EKI__DECK_HPP
#define MUIKA__MODULES__JQFTU__DECK_FOUNDATION__JAPAN_EKI__DECK_HPP

#include "muika/modules/jqftu/Deck.hpp"
#include "muika/modules/jqftu/deck_foundations/japan_eki/Card.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace deck_foundations {
namespace japan_eki {

class Deck: public muika::modules::jqftu::Deck {
private:
	static std::unique_ptr<Card> loadCardFromJson(const json &j);

protected:
	void loadDeckFromJsonFile(const char *file_path);

public:
	Deck(void) = default;

	virtual json toJson(void) const override;
	virtual void fromJson(const json &j) override;
};

} /* namespace muika::modules::jqftu::deck_foundations::japan_eki */
} /* namespace muika::modules::jqftu::deck_foundations */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__DECK_FOUNDATION__JAPAN_EKI__DECK_HPP */
