// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__INTERNAL_HPP
#define MUIKA__MODULES__M002_JQFTU__INTERNAL_HPP

#include <cstdio>

#ifndef pr_debug
#if 0
#define pr_debug(fmt, ...) printf("jqftu: " fmt "\n", ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) do {} while (0)
#endif
#endif

#endif /* MUIKA__MODULES__M002_JQFTU__INTERNAL_HPP */
