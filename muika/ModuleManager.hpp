// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULEMANAGER_HPP
#define MUIKA__MODULEMANAGER_HPP

#include <tgbot/tgbot.h>

#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace muika {

class Muika;

typedef std::function<int (muika::Muika &m, void **data)> mod_init_func_t;
typedef std::function<int (muika::Muika &m, TgBot::Message::Ptr &msg, void *data)> mod_entry_func_t;
typedef std::function<void (muika::Muika &m, void *data)> mod_free_func_t;

struct Module {
	const char		*name;
	int			init_ret;
	mod_init_func_t		f_init;
	mod_entry_func_t	f_entry;
	mod_free_func_t		f_free;
	void			*data;
};

class ModuleManager {
private:
	Muika *m_;
	std::vector<Module> modules_;

	void initModules(void);
	void freeModules(void);

public:
	ModuleManager(Muika *m);
	~ModuleManager(void);

	void handleMessage(TgBot::Message::Ptr &msg);
};

} /* namespace muika */

#endif /* #ifndef MUIKA__MODULEMANAGER_HPP */
