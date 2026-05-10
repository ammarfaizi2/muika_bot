// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/Module.hpp>

#include <cstring>

namespace Muika {

void Module::setModName(const char *name)
{
	strncpy(mod_name_, name, sizeof(mod_name_) - 1);
	mod_name_[sizeof(mod_name_) - 1] = '\0';
}

} /* namespace Muika */
