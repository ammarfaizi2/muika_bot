// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/entry.hpp>

namespace muika {
namespace modules {
namespace jqftu {

int mod_init(muika::Muika &m, void **data)
{
	return 0;
}

int mod_entry(muika::Muika &m, TgBot::Message::Ptr &msg, void *data)
{
	return 0;
}

void mod_free(muika::Muika &m, void *data)
{
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
