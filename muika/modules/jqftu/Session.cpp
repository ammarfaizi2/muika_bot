// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/Session.hpp>

namespace muika {
namespace modules {
namespace jqftu {

Session::Session(Muika& muika, int64_t chat_id, const std::vector<std::string> &deck_list):
	m_(muika),
	chat_id_(chat_id)
{
}

Session::~Session(void) = default;

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
