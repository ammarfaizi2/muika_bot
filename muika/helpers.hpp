// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__HELPERS_HPP
#define MUIKA__HELPERS_HPP

#include <string>
#include <cctype>

void mk_strtolower(std::string &s);
int mk_strcmpi(const char *s1, const char *s2, int n);
int mk_strncmpi(const char *s1, const char *s2, int n);
int mk_mkdir_p(const char *path, unsigned int mode);
int mk_file_get_contents(const std::string &path, std::string &out);
int mk_file_put_contents(const std::string &path, const std::string &data);

void mk_str_replace(std::string &str, const std::string &from,
		    const std::string &to, std::size_t *count = nullptr);
std::string mk_normalize_answer(const std::string &str);
std::string mk_htmlspecialchars(const std::string &str);
std::size_t mk_similar_text(const std::string &a, const std::string &b);

#endif /* #ifndef MUIKA__HELPERS_HPP */
