// SPDX-License-Identifier: GPL-2.0-only

#include <muika/helpers.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <sys/file.h>
#include <dirent.h>
#include <fcntl.h>

namespace muika {

void ____pr_debug(const char *fmt, ...)
{
	char *ptr, *heap = nullptr;
	va_list ap, ap2;
	char buf[1024];
	int ret;

	ptr = buf;

	va_start(ap, fmt);
	va_copy(ap2, ap);
	ret = vsnprintf(buf, sizeof(buf), fmt, ap);

	if (ret >= (int)sizeof(buf)) {
		heap = static_cast<char *>(malloc(ret + 1));
		if (heap) {
			ptr = heap;
			vsnprintf(heap, ret + 1, fmt, ap2);
		}
	}
	va_end(ap2);
	va_end(ap);

	printf("debug: %s\n", ptr);

	if (heap)
		free(heap);
}

std::vector<std::string>
str_explode(const std::string &str, const std::string &delim, size_t limit)
{
	std::vector<std::string> ret;
	size_t pos = 0, next;

	if (limit == 0)
		return ret;

	while (pos < str.size()) {
		next = str.find(delim, pos);
		if (next == std::string::npos) {
			ret.push_back(str.substr(pos));
			break;
		}

		ret.push_back(str.substr(pos, next - pos));
		pos = next + delim.size();

		if (limit != (size_t)-1 && ret.size() == limit - 1) {
			ret.push_back(str.substr(pos));
			break;
		}
	}

	return ret;
}

std::string file_get_contents(const std::string &filename)
{
	FILE *fp = nullptr;

	try {
		size_t len;

		fp = fopen(filename.c_str(), "rb");
		if (!fp)
			throw std::runtime_error("fopen failed: " + filename + ": " + strerror(errno));

		flock(fileno(fp), LOCK_EX);

		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		std::string ret(len, '\0');

		if (fread(&ret[0], 1, len, fp) != len)
			ret.clear();

		fclose(fp);
		return ret;
	} catch (...) {

		if (fp)
			fclose(fp);

		return "";
	}
}

void file_put_contents(const std::string &filename, const std::string &contents)
{
	FILE *fp;

	fp = fopen(filename.c_str(), "wb");
	if (!fp)
		throw std::runtime_error("fopen failed: " + filename + ": " + strerror(errno));

	flock(fileno(fp), LOCK_EX);
	fwrite(contents.c_str(), 1, contents.size(), fp);
	fclose(fp);
}

json json_file_get_contents(const std::string &filename)
{
	std::string contents = file_get_contents(filename);
	if (contents.empty())
		return json::object();

	return json::parse(contents);
}

void json_file_put_contents(const std::string &filename, const json &contents)
{
	file_put_contents(filename, contents.dump(4));
}

std::vector<std::string> scandir(const std::string &path, bool skip_dot)
{
	DIR *dir = nullptr;

	try {
		std::vector<std::string> ret;
		struct dirent *ent;

		dir = opendir(path.c_str());
		if (!dir)
			throw std::runtime_error("opendir failed: " + path + ": " + strerror(errno));

		while ((ent = readdir(dir))) {
			if (skip_dot && (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0))
				continue;

			ret.push_back(ent->d_name);
		}

		closedir(dir);
		dir = nullptr;
		return ret;
	} catch (std::exception &e) {
		if (dir)
			closedir(dir);

		throw e;
	}
}

void mkdir_recursive(const std::string &path_std, mode_t mode)
{
	char *path = strdup(path_std.c_str());
	int err;

	if (!path)
		throw std::runtime_error("strdup failed: " + path_std + ": " + strerror(errno));

	for (char *p = path + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			err = mkdir(path, mode);
			if (err && errno != EEXIST) {
				free(path);
				throw std::runtime_error("mkdir failed: " + path_std + ": " + strerror(errno));
			}
			*p = '/';
		}
	}

	err = mkdir(path, mode);
	if (err && errno != EEXIST) {
		free(path);
		throw std::runtime_error("mkdir failed: " + path_std + ": " + strerror(errno));
	}

	free(path);
}

bool is_number(const std::string &str)
{
	/*
	 * Allow negative numbers, but not positive numbers with a leading '+'.
	 */
	if (str.empty())
		return false;

	if (str[0] == '-')
		return str.size() > 1 && str.find_first_not_of("0123456789", 1) == std::string::npos;

	return str.find_first_not_of("0123456789") == std::string::npos;
}

void strtolower(std::string &str)
{
	for (char &c : str)
		c = tolower(c);
}

void strtoupper(std::string &str)
{
	for (char &c : str)
		c = toupper(c);
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

} /* namespace muika */
