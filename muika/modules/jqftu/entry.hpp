// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP
#define MUIKA__MODULES__JQFTU__ENTRY_HPP

#include <string>

#include "muika/ModuleManager.hpp"
#include "muika/modules/jqftu/Session.hpp"

namespace muika {
namespace modules {
namespace jqftu {

module_ret_t entry(muika::Muika &m, TgBot::Message::Ptr &msg);

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__ENTRY_HPP */
