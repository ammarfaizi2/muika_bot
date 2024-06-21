// SPDX-License-Identifier: GPL-2.0-only

#include <muika/ModuleManager.hpp>
#include <muika/Muika.hpp>

#include <muika/modules/jqftu/entry.hpp>

namespace muika {

ModuleManager::ModuleManager(Muika *m):
	m_(m)
{
	modules_ = {
		{
			.name		= "jqftu",
			.init_ret	= 0,
			.f_init		= &muika::modules::jqftu::mod_init,
			.f_entry	= &muika::modules::jqftu::mod_entry,
			.f_free		= &muika::modules::jqftu::mod_free,
			.data		= nullptr
		}
	};

	initModules();
}

ModuleManager::~ModuleManager(void)
{
	freeModules();
}

inline
void ModuleManager::initModules(void)
{
	for (auto &mod : modules_) {
		try {
			mod.init_ret = mod.f_init(*m_, &mod.data);
		} catch (const std::exception &e) {
			printf("ModuleManager::initModules: %s: %s\n", mod.name, e.what());
			mod.init_ret = -1;
		}
	}
}

inline
void ModuleManager::freeModules(void)
{
	for (auto &mod : modules_) {
		try {
			mod.f_free(*m_, mod.data);
		} catch (const std::exception &e) {
			printf("ModuleManager::freeModules: %s: %s\n", mod.name, e.what());
		}
	}
}

void ModuleManager::handleMessage(TgBot::Message::Ptr &msg)
{
	for (auto &mod : modules_) {

		if (mod.init_ret != 0)
			continue;

		try {
			if (!mod.f_entry(*m_, msg, mod.data))
				break;
		} catch (const std::exception &e) {
			printf("ModuleManager::handleMessage: %s: %s\n", mod.name, e.what());
		}
	}
}

} /* namespace muika */
