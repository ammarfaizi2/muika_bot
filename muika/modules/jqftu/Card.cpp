// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Card.hpp"

namespace muika {
namespace modules {
namespace jqftu {

static void strtolower(std::string &str)
{
	for (auto &c : str)
		c = tolower(c);
}

static void str_replace(std::string &str, const std::string &from,
			const std::string &to)
{
	size_t pos = 0;

	while ((pos = str.find(from, pos)) != std::string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

bool Card::normalizedJapaneseCompare(const std::string &a, const std::string &b) const
{
	if (a == b)
		return true;

	std::string aa = a, bb = b;

	strtolower(aa);
	strtolower(bb);
	if (aa == bb)
		return true;

	str_replace(aa, "-", "");
	str_replace(bb, "-", "");
	str_replace(aa, " ", "");
	str_replace(bb, " ", "");
	if (aa == bb)
		return true;

	std::string aa_o = aa, bb_o = bb;
	str_replace(aa_o, "ō", "o");
	str_replace(bb_o, "ō", "o");
	if (aa_o == bb_o)
		return true;

	std::string aa_ou = aa, bb_ou = bb;
	str_replace(aa_o, "ō", "ou");
	str_replace(bb_o, "ō", "ou");
	if (aa_ou == bb_ou)
		return true;

	std::string aa_u = aa, bb_u = bb;
	str_replace(aa_u, "ū", "u");
	str_replace(bb_u, "ū", "u");
	if (aa_u == bb_u)
		return true;

	std::string aa_uu = aa, bb_uu = bb;
	str_replace(aa_uu, "ū", "uu");
	str_replace(bb_uu, "ū", "uu");
	if (aa_uu == bb_uu)
		return true;

	std::string aa_n = aa, bb_n = bb;
	str_replace(aa_n, "n", "nn");
	str_replace(bb_n, "n", "nn");
	if (aa_n == bb_n)
		return true;

	return false;
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
