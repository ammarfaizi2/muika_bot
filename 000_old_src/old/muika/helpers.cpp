// SPDX-License-Identifier: GPL-2.0-only

#include "muika/helpers.hpp"

#include <string>
#include <cctype>

namespace muika {

void strtolower(std::string &str)
{
	for (auto &c : str)
		c = std::tolower(c);
}

void strtoupper(std::string &str)
{
	for (auto &c : str)
		c = std::toupper(c);
}

void str_replace(std::string &str, const std::string &from,
		 const std::string &to, size_t *count)
{
	size_t pos = 0;

	while ((pos = str.find(from, pos)) != std::string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();

		if (count)
			(*count)++;
	}
}

static void remove_chars(std::string &str, const std::string &chars)
{
	for (auto &c : chars)
		str_replace(str, std::string(1, c), "");
}

/*
 * Big thanks to PHP:
 * https://github.com/php/php-src/blob/b3bdc23656116f6f281385c05ead1a5d0b9aea6e/ext/standard/string.c#L3321-L3364
 */
static void php_similar_str(const char *txt1, size_t len1, const char *txt2, size_t len2, size_t *pos1, size_t *pos2, size_t *max, size_t *count)
{
	const char *p, *q;
	const char *end1 = (char *) txt1 + len1;
	const char *end2 = (char *) txt2 + len2;
	size_t l;

	*max = 0;
	*count = 0;
	for (p = (char *) txt1; p < end1; p++) {
		for (q = (char *) txt2; q < end2; q++) {
			for (l = 0; (p + l < end1) && (q + l < end2) && (p[l] == q[l]); l++);
			if (l > *max) {
				*max = l;
				*count += 1;
				*pos1 = p - txt1;
				*pos2 = q - txt2;
			}
		}
	}
}

static size_t php_similar_char(const char *txt1, size_t len1, const char *txt2, size_t len2)
{
	size_t sum;
	size_t pos1 = 0, pos2 = 0, max, count;

	php_similar_str(txt1, len1, txt2, len2, &pos1, &pos2, &max, &count);
	if ((sum = max)) {
		if (pos1 && pos2 && count > 1) {
			sum += php_similar_char(txt1, pos1, txt2, pos2);
		}
		if ((pos1 + max < len1) && (pos2 + max < len2)) {
			sum += php_similar_char(txt1 + pos1 + max, len1 - pos1 - max, txt2 + pos2 + max, len2 - pos2 - max);
		}
	}

	return sum;
}

size_t similar_text(const std::string &a, const std::string &b)
{
	size_t ret;

	ret = php_similar_char(a.c_str(), a.length(), b.c_str(), b.length());
	return (ret * 200) / (a.length() + b.length());
}

/*
 * Only alphanumeric characters, hyphens, quotes, and spaces are preserved.
 */
std::string normalize_answer(const std::string &str)
{
	std::string ret = str;

	str_replace(ret, "’", "'");
	str_replace(ret, "“", "\"");
	str_replace(ret, "”", "\"");
	str_replace(ret, "–", "-");
	str_replace(ret, "—", "-");
	str_replace(ret, "・", "-");
	str_replace(ret, "　", " ");
	remove_chars(ret, ".,:;!?()[]{}<>~`@#$%^&*+=|\\/");

	return ret;
}

std::string htmlspecialchars(const std::string &str)
{
	std::string ret = "";

	for (auto &c: str) {
		switch (c) {
		case '&':
			ret += "&amp;";
			break;
		case '<':
			ret += "&lt;";
			break;
		case '>':
			ret += "&gt;";
			break;
		case '"':
			ret += "&quot;";
			break;
		case '\'':
			ret += "&apos;";
			break;
		default:
			ret += c;
			break;
		}
	}
	return ret;
}

} /* namespace muika */
