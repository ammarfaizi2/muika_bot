// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULEMANAGER_HPP
#define MUIKA__MODULEMANAGER_HPP

#include "Muika.hpp"

#include <mutex>
#include <vector>
#include <functional>

namespace muika {

enum mod_init_ret_t {
	MOD_INIT_RET_OK    = 0,
	MOD_INIT_RET_ERR   = -1,
	MOD_INIT_RET_FATAL = -2,
	MOD_INIT_RET_UNDEF = -3,
};

enum mod_entry_ret_t {
	MOD_ENTRY_RET_CONTINUE = 0,
	MOD_ENTRY_RET_STOP     = -1,
};

struct ModuleState {
	void		*data;
	mod_init_ret_t	init_status;
};

typedef std::function<mod_init_ret_t (muika::Muika &m, void **data)> module_init_func_t;
typedef std::function<mod_entry_ret_t (muika::Muika &m, TgBot::Message::Ptr &msg, ModuleState *mod_state)> module_entry_func_t;
typedef std::function<void (muika::Muika &m, ModuleState *mod_state)> module_free_func_t;

struct Module {
	module_init_func_t	init;
	module_entry_func_t	entry;
	module_free_func_t	free;
};

class ModuleManager {
private:
	std::vector<Module> modules_;
	std::vector<ModuleState> module_states_;
	std::mutex mutex_;

public:
	ModuleManager(Module *modules, size_t nr_modules);
	~ModuleManager(void);
	int initModules(Muika &m);
	void invokeModules(Muika &m, TgBot::Message::Ptr &msg);
	void freeModules(Muika &m);
};

} /* namespace muika */

#endif /* MUIKA__MODULEMANAGER_HPP */
