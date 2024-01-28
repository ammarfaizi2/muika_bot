// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP
#define MUIKA__MODULES__JQFTU__ENTRY_HPP

#include "muika/ModuleManager.hpp"

namespace muika {
namespace modules {
namespace jqftu {

mod_init_ret_t init(muika::Muika &m, void **data);
mod_entry_ret_t entry(muika::Muika &m, TgBot::Message::Ptr &msg, ModuleState *mod_state);
void free(muika::Muika &m, ModuleState *mod_state);

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__ENTRY_HPP */
