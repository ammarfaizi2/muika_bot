// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/entry.hpp>
#include <muika/modules/jqftu/Session.hpp>
#include <muika/modules/jqftu/SessionMap.hpp>

namespace muika {
namespace modules {
namespace jqftu {

int mod_init(muika::Muika &m, void **data)
{
	SessionMap *smap = nullptr;
	int ret;

	try {
		smap = new SessionMap(m);
		smap->loadAll();
		ret = 0;
	} catch (std::bad_alloc &e) {
		ret = -1;
	} catch (std::exception &e) {
		ret = -1;
	} catch (...) {
		ret = -1;
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


	return 0;
}

void mod_free(muika::Muika &m, void *data)
{
	SessionMap *smap = static_cast<SessionMap *>(data);

	smap->save();
	delete smap;
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
