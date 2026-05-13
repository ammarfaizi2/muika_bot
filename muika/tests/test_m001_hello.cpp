// SPDX-License-Identifier: GPL-2.0-only
#include <muika/tests/MockReactor.hpp>
#include <muika/tests/test.hpp>
#include <muika/Muika.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>

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

} /* anonymous namespace */

MK_TEST(m001_hello, replies_to_slash_hello)
{
	std::string tmp = makeTmpDir("hello");
	auto reactor = std::make_shared<muika::tests::MockReactor>();

	{
		muika::MuikaConfig cfg;
		cfg.storage_path = tmp;
		muika::Muika mk(cfg, reactor);
		mk.loadModule("hello");

		auto msg = muika::Muika::createMsgText(
			"100", "1", "42", "Alice", "alice", "/hello");
		mk.passMsg(msg);
	}

	auto sent = reactor->drain();
	rmrf(tmp);

	MK_ASSERT_EQ(sent.size(), (std::size_t)1);
	MK_ASSERT(sent[0].kind == muika::tests::MockReactor::Kind::Text);
	MK_ASSERT_EQ(sent[0].chat_id, std::string("100"));
	MK_ASSERT_EQ(sent[0].om.text, std::string("Hello, Alice!"));
	MK_ASSERT_EQ(sent[0].om.reply_to_msg_id, std::string("1"));
}

MK_TEST(m001_hello, ignores_non_command)
{
	std::string tmp = makeTmpDir("hello");
	auto reactor = std::make_shared<muika::tests::MockReactor>();

	{
		muika::MuikaConfig cfg;
		cfg.storage_path = tmp;
		muika::Muika mk(cfg, reactor);
		mk.loadModule("hello");

		auto msg = muika::Muika::createMsgText(
			"100", "2", "42", "Alice", "alice", "hello there");
		mk.passMsg(msg);
	}

	auto sent = reactor->drain();
	rmrf(tmp);

	MK_ASSERT_EQ(sent.size(), (std::size_t)0);
}

MK_TEST(m001_hello, ignores_callback)
{
	std::string tmp = makeTmpDir("hello");
	auto reactor = std::make_shared<muika::tests::MockReactor>();

	{
		muika::MuikaConfig cfg;
		cfg.storage_path = tmp;
		muika::Muika mk(cfg, reactor);
		mk.loadModule("hello");

		auto msg = muika::Muika::createMsgCallback(
			"100", "3", "cb1", "42", "Alice", "alice", "/hello");
		mk.passMsg(msg);
	}

	auto sent = reactor->drain();
	rmrf(tmp);

	MK_ASSERT_EQ(sent.size(), (std::size_t)0);
}
