// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/entry.hpp>
#include <muika/modules/jqftu/Session.hpp>
#include <muika/modules/jqftu/SessionMap.hpp>
#include <muika/modules/jqftu/Command.hpp>

namespace muika {
namespace modules {
namespace jqftu {

int mod_init(muika::Muika &m, void **data)
{
	SessionMap *smap = nullptr;
	int ret;

	try {
		pr_debug("jqftu: Loading session map...");
		smap = new SessionMap(m);
		smap->loadAll();
		ret = 0;
	} catch (std::bad_alloc &e) {
		ret = -1;
		pr_debug("jqftu: Failed to allocate memory: %s", e.what());
	} catch (std::exception &e) {
		ret = -1;
		pr_debug("jqftu: Exception: %s", e.what());
	} catch (...) {
		ret = -1;
		pr_debug("jqftu: Unknown exception");
	}

	if (ret && smap)
		delete smap;

	if (ret == 0)
		*data = smap;

	return ret;
}

int mod_entry(muika::Muika &m, TgBot::Message::Ptr &msg, void *data)
{
	SessionMap *smap = static_cast<SessionMap *>(data);

	if (!smap)
		return -1;

	Command cmd(m, msg, *smap);
	if (cmd.handle())
		return 0;

	return -1;
}

void mod_free(muika::Muika &m, void *data)
{
	SessionMap *smap = static_cast<SessionMap *>(data);

	if (!smap)
		return;

	pr_debug("jqftu: Shutting down...");
	smap->save();
	delete smap;
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
