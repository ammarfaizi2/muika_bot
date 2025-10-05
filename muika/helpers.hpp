// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__HELPERS_HPP
#define MUIKA__HELPERS_HPP

#include <string>
#include <cctype>

void strtolower(std::string &s);
int strcmpi(const char *s1, const char *s2, int n);
int strncmpi(const char *s1, const char *s2, int n);

#endif /* #ifndef MUIKA__HELPERS_HPP */
