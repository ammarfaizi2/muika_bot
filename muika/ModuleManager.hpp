// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULEMANAGER_HPP
#define MUIKA__MODULEMANAGER_HPP

#include <tgbot/tgbot.h>

#include <string>
#include <memory>

namespace muika {

class Muika;

class ModuleManager {
private:
	Muika *m_;

public:
	ModuleManager(Muika *m);
	~ModuleManager(void);

	void handleMessage(TgBot::Message::Ptr &msg);
};

} /* namespace muika */

#endif /* #ifndef MUIKA__MODULEMANAGER_HPP */
