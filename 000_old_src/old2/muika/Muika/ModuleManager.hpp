// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA__MODULEMANAGER_HPP
#define MUIKA__MUIKA__MODULEMANAGER_HPP

#include <muika/Muika/Module.hpp>
#include <muika/Muika/MuikaBot.hpp>
#include <tgbot/tgbot.h>
#include <vector>
#include <memory>

namespace TgBot {
class Bot;
} /* namespace TgBot */

namespace Muika {
class ModuleManager {
public:
	ModuleManager(MuikaBot *mb);
	~ModuleManager(void);
	int invokeModules(TgBot::Message::Ptr &msg);

private:
	MuikaBot *mb_;
	std::vector<std::unique_ptr<Module>> modules_;

	void loadModule(std::unique_ptr<Module> mod);
	void loadModules(void);
	void unloadModules(void);
};

} /* namespace Muika */

#endif
