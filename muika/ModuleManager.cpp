// SPDX-License-Identifier: GPL-2.0-only

#include <muika/ModuleManager.hpp>

namespace muika {

ModuleManager::ModuleManager(Muika *m):
	m_(m)
{
}

ModuleManager::~ModuleManager(void) = default;

void ModuleManager::handleMessage(TgBot::Message::Ptr &msg)
{
}

} /* namespace muika */
