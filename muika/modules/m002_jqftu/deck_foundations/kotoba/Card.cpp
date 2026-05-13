// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/deck_foundations/kotoba/Card.hpp>
#include <muika/helpers.hpp>

#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace muika {
namespace modules {
namespace m002_jqftu {
namespace deck_foundations {
namespace kotoba {

static const char latex_template[] =
	"{\"content\":\"\\\\documentclass[32pt]{article}\\n"
	"\\\\usepackage{CJKutf8}\\n"
	"\\\\thispagestyle{empty}\\n"
	"\\\\begin{document}\\n"
	"\\\\begin{CJK}{UTF8}{min}\\n%s\\n"
	"\\\\end{CJK}\\n"
	"\\\\end{document}\\n\","
	"\"d\":800,\"border\":\"100x80\",\"bcolor\":\"white\"}";

bool Card::checkAnswer(const std::string &answer_ref)
{
	std::string answer = mk_normalize_answer(answer_ref);

	if (answer == kana_)
		return true;

	if (normalizedRomajiCompare(answer, romaji_))
		return true;

	return false;
}

std::string Card::getCardImage(void)
{
	std::string img_url;
	size_t len = sizeof(latex_template) + kanji_.size() + 1;
	char *tmp = (char *)malloc(len);
	if (!tmp)
		throw std::runtime_error("Failed to allocate memory");

	try {
		snprintf(tmp, len, latex_template, kanji_.c_str());
		img_url = generateLatexPngUrl(tmp);
	} catch (...) {
		free(tmp);
		throw;
	}
	free(tmp);
	return img_url;
}

std::string Card::getCardCaption(void)
{
	return "Write the kana or romaji for the following word!";
}

std::string Card::getCardAnswer(void)
{
	return kana_;
}

std::string Card::getCardDetails(void)
{
	return  "<b>Q:</b> " + kanji_ + "\n" +
		"<b>English:</b> " + en_ + "\n" +
		"<b>Answer: </b>" + kana_ + " (" + romaji_ + ")\n";
}

json Card::toJson(void) const
{
	json j;

	j["kanji"] = kanji_;
	j["kana"] = kana_;
	j["romaji"] = romaji_;
	j["en"] = en_;
	return j;
}

void Card::fromJson(const json &j)
{
	if (!j.is_object())
		throw std::runtime_error("Card JSON is not an object");
	if (!j.contains("kanji") || !j["kanji"].is_string())
		throw std::runtime_error("No string \"kanji\" key in JSON");
	if (!j.contains("kana") || !j["kana"].is_string())
		throw std::runtime_error("No string \"kana\" key in JSON");
	if (!j.contains("romaji") || !j["romaji"].is_string())
		throw std::runtime_error("No string \"romaji\" key in JSON");
	if (!j.contains("en") || !j["en"].is_string())
		throw std::runtime_error("No string \"en\" key in JSON");

	kanji_ = j["kanji"].get<std::string>();
	kana_ = j["kana"].get<std::string>();
	romaji_ = j["romaji"].get<std::string>();
	en_ = j["en"].get<std::string>();
}

} /* namespace kotoba */
} /* namespace deck_foundations */
} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
