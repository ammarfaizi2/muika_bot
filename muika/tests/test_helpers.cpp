// SPDX-License-Identifier: GPL-2.0-only
#include <muika/tests/test.hpp>
#include <muika/helpers.hpp>

#include <string>

MK_TEST(helpers, strtolower_basic)
{
	std::string s = "Hello, World!";
	mk_strtolower(s);
	MK_ASSERT_EQ(s, std::string("hello, world!"));
}

MK_TEST(helpers, strtolower_empty)
{
	std::string s;
	mk_strtolower(s);
	MK_ASSERT(s.empty());
}

MK_TEST(helpers, htmlspecialchars)
{
	MK_ASSERT_EQ(mk_htmlspecialchars("<b>&\"'"),
		     std::string("&lt;b&gt;&amp;&quot;&apos;"));
}

MK_TEST(helpers, str_replace)
{
	std::string s = "foo bar foo";
	mk_str_replace(s, "foo", "baz");
	MK_ASSERT_EQ(s, std::string("baz bar baz"));
}

MK_TEST(helpers, normalize_strips_punctuation)
{
	std::string in = "hello, world!?";
	MK_ASSERT_EQ(mk_normalize_answer(in), std::string("hello world"));
}
