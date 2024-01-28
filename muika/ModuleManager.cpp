// SPDX-License-Identifier: GPL-2.0-only

#include "ModuleManager.hpp"
#include "modules/jqftu/entry.hpp"

#include <cstdlib>
#include <functional>

namespace muika {

ModuleManager::ModuleManager(Module *modules, size_t nr_modules)
{
	size_t i;

	modules_.reserve(nr_modules);
	module_states_.reserve(nr_modules);

	for (i = 0; i < nr_modules; i++) {
		modules_.push_back(modules[i]);
		module_states_.push_back(ModuleState());
	}
}

ModuleManager::~ModuleManager(void) = default;

int ModuleManager::initModules(Muika &m)
{
	std::unique_lock<std::mutex> lock(mutex_);
	size_t i;

	for (i = 0; i < modules_.size(); i++) {
		mod_init_ret_t ret;

		if (modules_[i].init == nullptr)
			continue;

		ret = modules_[i].init(m, &module_states_[i].data);
		module_states_[i].init_status = ret;
		if (ret == MOD_INIT_RET_FATAL)
			return -1;
	}

	return 0;
}

void ModuleManager::invokeModules(Muika &m, TgBot::Message::Ptr &msg)
{
	std::unique_lock<std::mutex> lock(mutex_);
	size_t i;

	for (i = 0; i < modules_.size(); i++) {
		mod_entry_ret_t ret;

		if (modules_[i].entry == nullptr)
			continue;

		if (module_states_[i].init_status != MOD_INIT_RET_OK)
			continue;

		ret = modules_[i].entry(m, msg, &module_states_[i]);
		if (ret == MOD_ENTRY_RET_STOP)
			break;
	}
}

void ModuleManager::freeModules(Muika &m)
{
	std::unique_lock<std::mutex> lock(mutex_);
	size_t i;

	for (i = 0; i < modules_.size(); i++) {
		if (modules_[i].free == nullptr)
			continue;

		if (module_states_[i].init_status != MOD_INIT_RET_OK)
			continue;

		modules_[i].free(m, &module_states_[i]);
	}

	modules_ = std::vector<Module>();
	module_states_ = std::vector<ModuleState>();
}

} /* namespace muika */
