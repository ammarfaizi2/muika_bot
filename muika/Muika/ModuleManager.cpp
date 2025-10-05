// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/ModuleManager.hpp>
#include <muika/Muika/Modules/Jqftu/ModJqftu.hpp>
#include <cassert>

using namespace Muika::Modules;

namespace Muika {

ModuleManager::ModuleManager(MuikaBot *mb):
	mb_(mb)
{
	loadModules();
}

ModuleManager::~ModuleManager(void)
{
	unloadModules();
}

int ModuleManager::invokeModules(TgBot::Message::Ptr &msg)
{
	for (auto &mod : modules_) {
		int ret = mod->invoke(msg);

		if (ret == MOD_INVOKE_OK)
			return 0;
		if (ret == MOD_INVOKE_ERR)
			return -1;
		if (ret == MOD_INVOKE_SKIP)
			continue;

		/* Unknown return value */
		assert(false && "Unknown return value from module");
		return -1;
	}

	return 0;
}

inline void ModuleManager::loadModule(std::unique_ptr<Module> mod)
{
	if (mod->init())
		return;

	pr_info("Loaded module: %s", mod->getName());
	modules_.push_back(std::move(mod));
}

inline void ModuleManager::loadModules(void)
{
	pr_info("Loading modules...");
	loadModule(std::make_unique<Jqftu::ModJqftu>(mb_));
}

inline void ModuleManager::unloadModules(void)
{
	for (auto &mod : modules_) {
		pr_info("Shutting down module: %s", mod->getName());
		mod->shutdown();
	}

	modules_.clear();
}

} /* namespace Muika */
