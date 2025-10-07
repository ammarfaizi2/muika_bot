// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__HELPERS_HPP
#define MUIKA__HELPERS_HPP

#include <string>
#include <cctype>

void strtolower(std::string &s);
int strcmpi(const char *s1, const char *s2, int n);
int strncmpi(const char *s1, const char *s2, int n);
int mkdir_p(const char *path, mode_t mode);
int file_get_contents(const std::string &path, std::string &out);
int file_put_contents(const std::string &path, const std::string &data);

#endif /* #ifndef MUIKA__HELPERS_HPP */
