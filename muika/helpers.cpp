// SPDX-License-Identifier: GPL-2.0-only

#include <muika/helpers.hpp>

#include <cstring>
#include <cctype>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

void mk_strtolower(std::string &s)
{
	size_t i, l = s.length();
	for (i = 0; i < l; i++)
		s[i] = tolower(s[i]);
}

int mk_strcmpi(const char *s1, const char *s2, int n)
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

int mk_strncmpi(const char *s1, const char *s2, int n)
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

int mk_mkdir_p(const char *path, unsigned int mode)
{
	char *pp;
	char *sp;
	int status;
	char *copypath = strdup(path);
	if (!copypath)
		return -1;

	status = 0;
	pp = copypath;
	while (status == 0 && (sp = strchr(pp, '/')) != 0) {
		if (sp != pp) {
			*sp = '\0';
			if (mkdir(copypath, mode) != 0) {
				if (errno != EEXIST)
					status = -1;
			}
			*sp = '/';
		}
		pp = sp + 1;
	}

	if (status == 0 && *pp != '\0') {
		if (mkdir(copypath, mode) != 0) {
			if (errno != EEXIST)
				status = -1;
		}
	}

	free(copypath);
	return status;
}

int mk_file_get_contents(const std::string &path, std::string &out)
{
	FILE *f = fopen(path.c_str(), "rb");
	if (!f)
		return -errno;

	out.clear();
	char buf[4096];
	size_t n;
	while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
		out.append(buf, n);

	if (ferror(f)) {
		int err = -EIO;
		fclose(f);
		return err;
	}

	fclose(f);
	return 0;
}

int mk_file_put_contents(const std::string &path, const std::string &data)
{
	FILE *f = fopen(path.c_str(), "wb");
	if (!f)
		return -errno;

	size_t n = fwrite(data.data(), 1, data.size(), f);
	if (n < data.size()) {
		int err = -EIO;
		fclose(f);
		return err;
	}

	if (fclose(f) != 0)
		return -EIO;

	return 0;
}

void mk_str_replace(std::string &str, const std::string &from,
		    const std::string &to, std::size_t *count)
{
	if (from.empty())
		return;

	std::size_t pos = 0;
	while ((pos = str.find(from, pos)) != std::string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
		if (count)
			(*count)++;
	}
}

static void mk_remove_chars(std::string &str, const std::string &chars)
{
	for (auto &c : chars)
		mk_str_replace(str, std::string(1, c), "");
}

/*
 * Port of PHP's similar_text(). Returns a 0..200 similarity score
 * (the upstream PHP function is 0..100; this scales by 200 because
 * the divisor is (a.len + b.len) rather than (a.len + b.len) / 2).
 *
 * Reference:
 *   https://github.com/php/php-src/blob/b3bdc23656116f6f281385c05ead1a5d0b9aea6e/ext/standard/string.c#L3321-L3364
 */
static void mk_php_similar_str(const char *t1, std::size_t l1,
			       const char *t2, std::size_t l2,
			       std::size_t *p1, std::size_t *p2,
			       std::size_t *max, std::size_t *count)
{
	const char *p, *q;
	const char *e1 = t1 + l1;
	const char *e2 = t2 + l2;
	std::size_t l;

	*max = 0;
	*count = 0;
	for (p = t1; p < e1; p++) {
		for (q = t2; q < e2; q++) {
			for (l = 0; (p + l < e1) && (q + l < e2) && (p[l] == q[l]); l++);
			if (l > *max) {
				*max = l;
				(*count)++;
				*p1 = p - t1;
				*p2 = q - t2;
			}
		}
	}
}

static std::size_t mk_php_similar_char(const char *t1, std::size_t l1,
				       const char *t2, std::size_t l2)
{
	std::size_t sum;
	std::size_t pos1 = 0, pos2 = 0, max = 0, count = 0;

	mk_php_similar_str(t1, l1, t2, l2, &pos1, &pos2, &max, &count);
	if ((sum = max)) {
		if (pos1 && pos2 && count > 1)
			sum += mk_php_similar_char(t1, pos1, t2, pos2);
		if ((pos1 + max < l1) && (pos2 + max < l2))
			sum += mk_php_similar_char(t1 + pos1 + max,
						   l1 - pos1 - max,
						   t2 + pos2 + max,
						   l2 - pos2 - max);
	}
	return sum;
}

std::size_t mk_similar_text(const std::string &a, const std::string &b)
{
	std::size_t total = a.length() + b.length();
	if (total == 0)
		return 0;

	std::size_t r = mk_php_similar_char(a.c_str(), a.length(),
					    b.c_str(), b.length());
	return (r * 200) / total;
}

/*
 * Lightweight whitespace / punctuation normaliser used to compare
 * romaji answers tolerantly.  Smart quotes / dashes / fullwidth
 * spaces are collapsed to ASCII; common punctuation is stripped.
 */
std::string mk_normalize_answer(const std::string &str)
{
	std::string ret = str;

	mk_str_replace(ret, "\xe2\x80\x99", "'");  /* ’ */
	mk_str_replace(ret, "\xe2\x80\x9c", "\""); /* “ */
	mk_str_replace(ret, "\xe2\x80\x9d", "\""); /* ” */
	mk_str_replace(ret, "\xe2\x80\x93", "-");  /* – */
	mk_str_replace(ret, "\xe2\x80\x94", "-");  /* — */
	mk_str_replace(ret, "\xe3\x83\xbb", "-");  /* ・ */
	mk_str_replace(ret, "\xe3\x80\x80", " ");  /* fullwidth space */
	mk_remove_chars(ret, ".,:;!?()[]{}<>~`@#$%^&*+=|\\/");

	return ret;
}

std::string mk_htmlspecialchars(const std::string &str)
{
	std::string ret;
	ret.reserve(str.size());

	for (auto &c : str) {
		switch (c) {
		case '&': ret += "&amp;";  break;
		case '<': ret += "&lt;";   break;
		case '>': ret += "&gt;";   break;
		case '"': ret += "&quot;"; break;
		case '\'': ret += "&apos;"; break;
		default: ret += c; break;
		}
	}
	return ret;
}
