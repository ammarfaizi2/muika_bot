// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/entry.hpp"
#include "muika/modules/jqftu/Command.hpp"
#include "muika/modules/jqftu/SessionMap.hpp"

#include <mutex>
#include <cctype>
#include <vector>
#include <memory>

namespace muika {
namespace modules {
namespace jqftu {

mod_init_ret_t init(muika::Muika &m, void **data)
{
	SessionMap *smap;

	try {
		smap = new SessionMap();
		smap->loadSessionsFromDisk(m);
	} catch (std::bad_alloc &e) {
		return MOD_INIT_RET_ERR;
	}

	*data = static_cast<void *>(smap);
	return MOD_INIT_RET_OK;
}

mod_entry_ret_t entry(muika::Muika &m, TgBot::Message::Ptr &msg, ModuleState *mod_state)
{
	SessionMap *smap;

	if (mod_state->init_status != MOD_INIT_RET_OK)
		return MOD_ENTRY_RET_CONTINUE;

	smap = static_cast<SessionMap *>(mod_state->data);
	Command cmd(m, msg, smap);
	if (cmd.execute())
		return MOD_ENTRY_RET_STOP;

	return MOD_ENTRY_RET_CONTINUE;
}

void free(muika::Muika &m, ModuleState *mod_state)
{
	SessionMap *smap;

	if (mod_state->init_status != MOD_INIT_RET_OK)
		return;

	smap = static_cast<SessionMap *>(mod_state->data);
	delete smap;
	(void)m;
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
