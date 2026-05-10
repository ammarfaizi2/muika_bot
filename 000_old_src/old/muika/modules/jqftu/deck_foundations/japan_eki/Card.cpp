// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/deck_foundations/japan_eki/Card.hpp"
#include "muika/helpers.hpp"

#include <cstring>
#include <cstdlib>
#include <cstdio>

using muika::normalize_answer;

namespace muika {
namespace modules {
namespace jqftu {
namespace deck_foundations {
namespace japan_eki {

static const char latex_template[] =  "{\"content\":\"\\\\documentclass[32pt]{article}\\n\\\\usepackage{CJKutf8}\\n\\\\thispagestyle{empty}\\n\\\\begin{document}\\n\\\\begin{CJK}{UTF8}{min}\\n%s\\n\\\\end{CJK}\\n\\\\end{document}\\n\",\"d\":800,\"border\":\"100x80\",\"bcolor\":\"white\"}";

bool Card::checkAnswer(const std::string &answer_ref)
{
	std::string answer = normalize_answer(answer_ref);

	if (answer == hiragana_ || answer == katakana_)
		return true;

	if (normalizedRomajiCompare(answer, romaji_))
		return true;

	return false;
}

std::string Card::getCardImage(void)
{
	std::string img_url;
	size_t len;
	char *tmp;

	len = sizeof(latex_template) + strlen(kanji_.c_str()) + 1;
	tmp = (char *)malloc(len);
	if (!tmp)
		throw std::runtime_error("Failed to allocate memory");

	try {
		snprintf(tmp, len, latex_template, kanji_.c_str());
		img_url = generateLatexPngUrl(tmp);
		free(tmp);
	} catch (const std::exception &e) {
		free(tmp);
		throw e;
	}

	return img_url;
}

std::string Card::getCardCaption(void)
{
	return "Write the kana or romaji for the following station!";
}

std::string Card::getCardAnswer(void)
{
	return hiragana_;
}

std::string Card::getCardDetails(void)
{
	return  "<b>Q:</b> " + kanji_ + "\n" +
		"<b>Station Number:</b> " + n_ + "\n" +
		"<b>Answer: </b>" + hiragana_ + " (" + romaji_ + ")\n";
}

json Card::toJson(void) const
{
	json j;

	j["n"] = n_;
	j["kanji"] = kanji_;
	j["romaji"] = romaji_;
	j["hiragana"] = hiragana_;
	j["katakana"] = katakana_;
	return j;
}

void Card::fromJson(const json &j)
{
	if (!j.contains("n") || !j["n"].is_string())
		throw std::runtime_error("No string \"n\" key in JSON");

	if (!j.contains("kanji") || !j["kanji"].is_string())
		throw std::runtime_error("No string \"kanji\" key in JSON");

	if (!j.contains("romaji") || !j["romaji"].is_string())
		throw std::runtime_error("No string \"romaji\" key in JSON");

	if (!j.contains("hiragana") || !j["hiragana"].is_string())
		throw std::runtime_error("No string \"hiragana\" key in JSON");

	if (!j.contains("katakana") || !j["katakana"].is_string())
		throw std::runtime_error("No string \"katakana\" key in JSON");

	n_ = j["n"];
	kanji_ = j["kanji"];
	romaji_ = j["romaji"];
	hiragana_ = j["hiragana"];
	katakana_ = j["katakana"];
}

} /* namespace muika::modules::jqftu::deck_foundations::japan_eki */
} /* namespace muika::modules::jqftu::deck_foundations */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
