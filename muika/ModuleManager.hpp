// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULEMANAGER_HPP
#define MUIKA__MODULEMANAGER_HPP

#include "Muika.hpp"

#include <functional>

enum module_ret_t {
	/*
	 * Continue invoking other modules.
	 */
	MOD_ENTRY_CONTINUE   = 0,

	/*
	 * Stop invoking other modules.
	 */
	MOD_ENTRY_STOP       = 1,
};

typedef std::function<module_ret_t (muika::Muika &m, TgBot::Message::Ptr &msg)> module_entry_t;
typedef std::function<module_ret_t (muika::Muika &m)> module_init_t;

namespace muika {

class ModuleManager {
public:
	static int initModules(Muika &m);
	static int invokeModules(Muika &m, TgBot::Message::Ptr &msg);
};

} /* namespace muika */

#endif /* MUIKA__MODULEMANAGER_HPP */
