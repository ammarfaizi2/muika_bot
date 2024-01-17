// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__INTERNAL_HPP
#define MUIKA__MODULES__JQFTU__INTERNAL_HPP

#include "Session.hpp"

#include <mutex>
#include <unordered_map>

namespace muika {
namespace modules {
namespace jqftu {

#if 1
#define pr_debug(fmt, ...) printf("jqftu: " fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) do {} while (0)
#endif

extern std::unordered_map<int64_t, Session *> sessions;
extern std::mutex sessions_mutex;

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__INTERNAL_HPP */
