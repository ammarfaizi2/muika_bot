// SPDX-License-Identifier: GPL-2.0-only

#include <muika/helpers.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include <sys/file.h>

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
		heap = (char *)malloc(ret + 1);
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
			return "";

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
		return;

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

} /* namespace muika */
