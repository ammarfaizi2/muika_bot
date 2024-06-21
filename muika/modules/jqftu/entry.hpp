// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP
#define MUIKA__MODULES__JQFTU__ENTRY_HPP

#include <muika/ModuleManager.hpp>

namespace muika {
namespace modules {
namespace jqftu {

int mod_init(muika::Muika &m, void **data);
int mod_entry(muika::Muika &m, TgBot::Message::Ptr &msg, void *data);
void mod_free(muika::Muika &m, void *data);

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP */
