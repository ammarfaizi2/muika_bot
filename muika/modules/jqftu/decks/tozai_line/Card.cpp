// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/decks/tozai_line/Card.hpp"

namespace muika {
namespace modules {
namespace jqftu {
namespace decks {
namespace tozai_line {

bool Card::answer(const std::string &answer) const
{
	return false;
}

const std::string &Card::getInfo(void) const
{
	return n_;
}

} /* namespace muika::modules::jqftu::decks::tozai_line */
} /* namespace muika::modules::jqftu::decks */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
