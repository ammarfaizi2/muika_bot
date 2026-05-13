// SPDX-License-Identifier: GPL-2.0-only

#include <muika/helpers.hpp>
#include <cstring>
#include <cctype>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

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

int mkdir_p(const char *path, mode_t mode)
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

int file_get_contents(const std::string &path, std::string &out)
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

int file_put_contents(const std::string &path, const std::string &data)
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
