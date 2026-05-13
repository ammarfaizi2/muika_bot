// SPDX-License-Identifier: GPL-2.0-only
#include <muika/tests/MockReactor.hpp>
#include <muika/tests/test.hpp>
#include <muika/Muika.hpp>
#include <muika/modules/m002_jqftu/DeckGroup.hpp>
#include <muika/modules/m002_jqftu/Point.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

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

const std::string repo_decks_dir = "./storage/mtgbot/jqftu/decks";

} /* anonymous namespace */

namespace m002 = muika::modules::m002_jqftu;

MK_TEST(m002_jqftu_point, json_roundtrip)
{
	m002::Point p(42, 7, "alice", "Alice", "Wonder");
	std::string s = p.toJsonString();

	m002::Point q;
	q.fromJsonString(s);

	MK_ASSERT_EQ(q.getUserId(), (uint64_t)42);
	MK_ASSERT_EQ(q.getPoint(), (uint64_t)7);
	MK_ASSERT_EQ(q.getUsername(), std::string("alice"));
	MK_ASSERT_EQ(q.getFirstName(), std::string("Alice"));
	MK_ASSERT_EQ(q.getLastName(), std::string("Wonder"));
	MK_ASSERT_EQ(q.getFullName(), std::string("Alice Wonder"));
}

MK_TEST(m002_jqftu_point, save_load_disk)
{
	std::string tmp = makeTmpDir("point");

	m002::Point p(123, 9, "bob", "Bob", "");
	p.saveToDisk(tmp, 555);

	m002::Point loaded = m002::Point::tryLoadFromDisk(tmp, 555, 123);
	std::string scoreboard_dir = m002::Point::chatPointsDir(tmp, 555);

	rmrf(tmp);

	MK_ASSERT_EQ(loaded.getUserId(), (uint64_t)123);
	MK_ASSERT_EQ(loaded.getPoint(), (uint64_t)9);
	MK_ASSERT_EQ(loaded.getFirstName(), std::string("Bob"));
	MK_ASSERT_EQ(scoreboard_dir, tmp + "/s_555");
}

MK_TEST(m002_jqftu_point, missing_file_returns_blank)
{
	std::string tmp = makeTmpDir("point");

	m002::Point loaded = m002::Point::tryLoadFromDisk(tmp, 1, 99999);

	rmrf(tmp);

	MK_ASSERT_EQ(loaded.getUserId(), (uint64_t)0);
	MK_ASSERT_EQ(loaded.getPoint(), (uint64_t)0);
}

MK_TEST(m002_jqftu_point, accumulate)
{
	m002::Point p(1, 0, "u", "U", "");
	p.addPoint(5);
	p.addPoint(3);
	MK_ASSERT_EQ(p.getPoint(), (uint64_t)8);
	p.subPoint(2);
	MK_ASSERT_EQ(p.getPoint(), (uint64_t)6);
}

MK_TEST(m002_jqftu_deckgroup, create_each_known_deck)
{
	m002::DeckGroup dg(repo_decks_dir);

	const char *names[] = {
		"tozai_line", "yamanote_line", "chuo_sobu",
		"keikyu_line", "jlpt_n5",
	};

	for (const char *n : names) {
		auto deck = dg.createDeckByName(n);
		MK_ASSERT(deck != nullptr);
		MK_ASSERT_EQ(deck->getName(), std::string(n));
		MK_ASSERT(deck->getNumberOfCards() > 0u);
	}
}

MK_TEST(m002_jqftu_deckgroup, unknown_deck_throws)
{
	m002::DeckGroup dg(repo_decks_dir);

	bool threw = false;
	try {
		dg.createDeckByName("does_not_exist");
	} catch (const std::exception &) {
		threw = true;
	}
	MK_ASSERT_TRUE(threw);
}

MK_TEST(m002_jqftu_deckgroup, draw_then_exhaust)
{
	m002::DeckGroup dg(repo_decks_dir);
	dg.addDeckByName("tozai_line");

	auto *deck = dg.drawDeck();
	MK_ASSERT(deck != nullptr);

	uint32_t total = deck->getNumberOfCards();
	MK_ASSERT(total > 0u);

	uint32_t drawn = 0;
	while (deck->drawCard()) {
		++drawn;
		if (drawn > total + 1)
			break;
	}
	MK_ASSERT_EQ(drawn, total);
}

MK_TEST(m002_jqftu_module, help_command)
{
	std::string tmp = makeTmpDir("jqftu");

	/* Stage decks under the per-module storage dir. */
	std::string mod_decks = tmp + "/jqftu/decks";
	mkdir((tmp + "/jqftu").c_str(), 0755);
	mkdir(mod_decks.c_str(), 0755);

	auto reactor = std::make_shared<muika::tests::MockReactor>();

	{
		muika::MuikaConfig cfg;
		cfg.storage_path = tmp;
		muika::Muika mk(cfg, reactor);
		mk.loadModule("jqftu");

		auto msg = muika::Muika::createMsgText(
			"100", "1", "42", "Alice", "alice", "/jqftu help");
		mk.passMsg(msg);
	}

	auto sent = reactor->drain();
	rmrf(tmp);

	MK_ASSERT_EQ(sent.size(), (std::size_t)1);
	MK_ASSERT(sent[0].kind == muika::tests::MockReactor::Kind::Text);
	MK_ASSERT(sent[0].om.text.find("Japanese quiz module") !=
		  std::string::npos);
}

MK_TEST(m002_jqftu_module, set_default_validation)
{
	std::string tmp = makeTmpDir("jqftu");

	std::string mod_decks = tmp + "/jqftu/decks";
	mkdir((tmp + "/jqftu").c_str(), 0755);
	mkdir(mod_decks.c_str(), 0755);

	auto reactor = std::make_shared<muika::tests::MockReactor>();

	{
		muika::MuikaConfig cfg;
		cfg.storage_path = tmp;
		muika::Muika mk(cfg, reactor);
		mk.loadModule("jqftu");

		auto bad = muika::Muika::createMsgText(
			"100", "1", "42", "Alice", "alice",
			"/jqftu set default.timeout abc");
		mk.passMsg(bad);

		auto unknown = muika::Muika::createMsgText(
			"100", "2", "42", "Alice", "alice",
			"/jqftu set wrong.key 100");
		mk.passMsg(unknown);

		auto ok = muika::Muika::createMsgText(
			"100", "3", "42", "Alice", "alice",
			"/jqftu set default.timeout 1234");
		mk.passMsg(ok);

		auto cur_no_session = muika::Muika::createMsgText(
			"100", "4", "42", "Alice", "alice",
			"/jqftu set cur.timeout 60");
		mk.passMsg(cur_no_session);
	}

	auto sent = reactor->drain();

	/* Verify the persisted config file. */
	std::string cfg_path = tmp + "/jqftu/config/s_100.json";
	FILE *f = fopen(cfg_path.c_str(), "rb");
	bool cfg_exists = (f != nullptr);
	if (f)
		fclose(f);

	rmrf(tmp);

	MK_ASSERT_EQ(sent.size(), (std::size_t)4);
	MK_ASSERT(sent[0].om.text.find("Invalid value") != std::string::npos);
	MK_ASSERT(sent[1].om.text.find("Unknown set key") != std::string::npos);
	MK_ASSERT(sent[2].om.text.find("Set") != std::string::npos);
	MK_ASSERT(sent[2].om.text.find("1234") != std::string::npos);
	MK_ASSERT(sent[3].om.text.find("No quiz is running") != std::string::npos);
	MK_ASSERT_TRUE(cfg_exists);
}
