// SPDX-License-Identifier: GPL-2.0-only

#include <muika/helpers.hpp>

void strtolower(std::string &s)
{
	size_t i, l = s.length();
	for (i = 0; i < l; i++)
		s[i] = tolower(s[i]);
}

int strcmpi(const char *s1, const char *s2, int n)
{
	int c1, c2;
	while (n--) {
		c1 = tolower((unsigned char)*s1++);
		c2 = tolower((unsigned char)*s2++);
		if (c1 != c2)
			return c1 - c2;
		if (!c1)
			break;
	}
	return 0;
}

int strncmpi(const char *s1, const char *s2, int n)
{
	int c1, c2;
	while (n--) {
		c1 = tolower((unsigned char)*s1++);
		c2 = tolower((unsigned char)*s2++);
		if (c1 != c2)
			return c1 - c2;
		if (!c1)
			break;
	}
	return 0;
}
