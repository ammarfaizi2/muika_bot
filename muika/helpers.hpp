// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__HELPERS_HPP
#define MUIKA__HELPERS_HPP

#include <nlohmann/json.hpp>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <cassert>

#ifndef __serialize
#define __serialize
#endif

#ifndef __must_hold
#define __must_hold(x)
#endif

namespace muika {

void ____pr_debug(const char *fmt, ...);
std::vector<std::string> str_explode(const std::string &str, const std::string &delim, size_t limit = -1);

} /* namespace muika */

#ifndef pr_debug
#define pr_debug(...) ::muika::____pr_debug(__VA_ARGS__)
#endif

#endif
