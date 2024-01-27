// SPDX-License-Identifier: GPL-2.0-only

#include "ModuleManager.hpp"
#include "modules/jqftu/entry.hpp"

#include <functional>

namespace muika {

static const module_init_t f_modules_init[] = {
	&modules::jqftu::init,
};

static const module_entry_t f_modules_entry[] = {
	&modules::jqftu::entry,
};

// static
int ModuleManager::initModules(Muika &m)
{
	size_t i, n = sizeof(f_modules_init) / sizeof(f_modules_init[0]);

	for (i = 0; i < n; i++) {
		if (!f_modules_init[i])
			continue;

		f_modules_init[i](m);
	}

	return 0;
}

// static
int ModuleManager::invokeModules(Muika &m, TgBot::Message::Ptr &msg)
{
	size_t i, n = sizeof(f_modules_entry) / sizeof(f_modules_entry[0]);
	module_ret_t ret;

	for (i = 0; i < n; i++) {
		if (!f_modules_entry[i])
			continue;

		ret = f_modules_entry[i](m, msg);
		if (ret == MOD_ENTRY_STOP)
			return 0;
	}

	return 0;
}

} /* namespace muika */
