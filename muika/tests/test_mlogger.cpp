// SPDX-License-Identifier: GPL-2.0-only
#include <muika/tests/test.hpp>
#include <mlogger/mlogger.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ftw.h>
#include <string>
#include <thread>
#include <vector>

namespace {

int rm_entry(const char *path, const struct stat *, int, struct FTW *)
{
	remove(path);
	return 0;
}

void rmrf(const std::string &path)
{
	nftw(path.c_str(), rm_entry, 16, FTW_DEPTH | FTW_PHYS);
}

std::string makeTmpDir(const char *tag)
{
	std::string tmpl = std::string("/tmp/muika_test_") + tag + "_XXXXXX";
	std::vector<char> buf(tmpl.begin(), tmpl.end());
	buf.push_back('\0');
	if (!mkdtemp(buf.data()))
		throw std::runtime_error("mkdtemp failed");
	return std::string(buf.data());
}

std::string slurp(const std::string &path)
{
	FILE *f = fopen(path.c_str(), "rb");
	if (!f)
		return std::string();
	std::string out;
	char buf[4096];
	size_t n;
	while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
		out.append(buf, n);
	fclose(f);
	return out;
}

} /* anonymous namespace */

MK_TEST(mlogger, init_and_write_to_file)
{
	std::string tmp = makeTmpDir("mlog");
	std::string path = tmp + "/test.log";

	mk_logger_t *l = mk_logger_init(path.c_str());
	MK_ASSERT(l != nullptr);

	mk_info(l, "hello %s %d", "world", 42);
	mk_logger_free(l);

	std::string contents = slurp(path);
	rmrf(tmp);

	MK_ASSERT(contents.find("[INFO] hello world 42") != std::string::npos);
	MK_ASSERT(contents.back() == '\n');
}

MK_TEST(mlogger, level_filtering)
{
	std::string tmp = makeTmpDir("mlog");
	std::string path = tmp + "/test.log";

	mk_logger_t *l = mk_logger_init(path.c_str());
	MK_ASSERT(l != nullptr);

	mk_logger_set_level(l, MK_LOG_WARN);
	MK_ASSERT_EQ(mk_logger_get_level(l), (int)MK_LOG_WARN);
	MK_ASSERT_FALSE(mk_logger_should_log(l, MK_LOG_INFO));
	MK_ASSERT_TRUE(mk_logger_should_log(l, MK_LOG_ERROR));

	mk_debug(l, "should not appear DBG");
	mk_info(l, "should not appear INFO");
	mk_warn(l, "warn-keep-me");
	mk_error(l, "err-keep-me");
	mk_logger_free(l);

	std::string c = slurp(path);
	rmrf(tmp);

	MK_ASSERT(c.find("DBG") == std::string::npos);
	MK_ASSERT(c.find("INFO") == std::string::npos);
	MK_ASSERT(c.find("warn-keep-me") != std::string::npos);
	MK_ASSERT(c.find("err-keep-me") != std::string::npos);
}

MK_TEST(mlogger, null_logger_is_safe)
{
	mk_info(nullptr, "no crash");
	mk_logger_flush(nullptr);
	mk_logger_set_level(nullptr, MK_LOG_INFO);
	MK_ASSERT_FALSE(mk_logger_should_log(nullptr, MK_LOG_INFO));
	mk_logger_free(nullptr);
}

MK_TEST(mlogger, thread_safe_writes)
{
	std::string tmp = makeTmpDir("mlog");
	std::string path = tmp + "/test.log";

	mk_logger_t *l = mk_logger_init(path.c_str());
	MK_ASSERT(l != nullptr);

	const int N = 8;
	const int M = 100;
	std::vector<std::thread> th;
	for (int i = 0; i < N; ++i) {
		th.emplace_back([l, i]() {
			for (int j = 0; j < M; ++j)
				mk_info(l, "t=%d j=%d", i, j);
		});
	}
	for (auto &t : th)
		t.join();

	mk_logger_free(l);

	std::string c = slurp(path);
	rmrf(tmp);

	int lines = 0;
	for (char ch : c)
		if (ch == '\n')
			++lines;
	MK_ASSERT_EQ(lines, N * M);
}

MK_TEST(mlogger, init_file_does_not_close_caller_fp)
{
	std::string tmp = makeTmpDir("mlog");
	std::string path = tmp + "/test.log";

	FILE *fp = fopen(path.c_str(), "wb");
	MK_ASSERT(fp != nullptr);

	mk_logger_t *l = mk_logger_init_file(fp);
	MK_ASSERT(l != nullptr);
	mk_warn(l, "via fp");
	mk_logger_free(l);

	/* Caller still owns fp; we should be able to write more. */
	fputs("trailing\n", fp);
	fclose(fp);

	std::string c = slurp(path);
	rmrf(tmp);
	MK_ASSERT(c.find("[WARN] via fp") != std::string::npos);
	MK_ASSERT(c.find("trailing") != std::string::npos);
}
