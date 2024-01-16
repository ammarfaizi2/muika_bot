// SPDX-License-Identifier: GPL-2.0-only

#include "ModuleManager.hpp"
#include "modules/jqftu/entry.hpp"

#include <functional>

namespace muika {

static const module_entry_t f_modules[] = {
	&modules::jqftu::entry,
};

// static
int ModuleManager::invokeModules(Muika &m, TgBot::Message::Ptr &msg)
{
	size_t i, n = sizeof(f_modules) / sizeof(f_modules[0]);
	module_ret_t ret;

	for (i = 0; i < n; i++) {
		if (!f_modules[i])
			continue;

		ret = f_modules[i](m, msg);
		if (ret == MOD_ENTRY_STOP)
			return 0;
	}

	return 0;
}

} /* namespace muika */
