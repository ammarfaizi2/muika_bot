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

	if (alt_.size() > 0) {
		for (const auto &alt: alt_) {
			if (normalizedRomajiCompare(answer, alt))
				return true;
		}
	}

	return false;
}

std::string Card::getCardImage(void)
{
	if (!q_img_.empty())
		return "https://telegram-bot.teainside.org/output/" + q_img_;

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
	std::string ret =
		"<b>Q:</b> " + kanji_ + "\n" +
		"<b>Station Number:</b> " + n_ + "\n" +
		"<b>Answer: </b>" + hiragana_ + " (" + romaji_ + ")\n";

	if (alt_.size() > 0) {
		ret += "\n<b>Alternate Answers:</b>\n";
		for (const auto &alt: alt_)
			ret += alt + "\n";
	}

	return ret;
}

json Card::toJson(void) const
{
	json j;

	j["n"] = n_;
	j["kanji"] = kanji_;
	j["romaji"] = romaji_;
	j["hiragana"] = hiragana_;
	j["katakana"] = katakana_;
	j["q_img"] = q_img_;
	j["photos"] = json::array();

	if (alt_.size() > 0) {
		j["alt"] = json::array();
		for (const auto &alt: alt_)
			j["alt"].push_back(alt);
	}

	if (photos_.size() > 0) {
		for (const auto &photo: photos_)
			j["photos"].push_back(photo);
	}

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

	if (j.contains("alt")) {
		if (!j["alt"].is_array())
			throw std::runtime_error("\"alt\" key is not an array");

		for (const auto &alt: j["alt"]) {
			if (!alt.is_string())
				throw std::runtime_error("Non-string value in \"alt\" array");
		}

		alt_.clear();
		for (const auto &alt: j["alt"])
			alt_.push_back(alt);
	}

	q_img_ = "";
	if (j.contains("q_img") && j["q_img"].is_string())
		q_img_ = j["q_img"];

	photos_.clear();
	if (j.contains("photos") && j["photos"].is_array()) {
		for (const auto &photo: j["photos"]) {
			if (!photo.is_string())
				throw std::runtime_error("Non-string value in \"photos\" array");
			photos_.push_back(photo);
		}
	}


	n_ = j["n"];
	kanji_ = j["kanji"];
	romaji_ = j["romaji"];
	hiragana_ = j["hiragana"];
	katakana_ = j["katakana"];
}

const std::vector<std::string> &Card::getCardPhotos(void)
{
	return photos_;
}

} /* namespace muika::modules::jqftu::deck_foundations::japan_eki */
} /* namespace muika::modules::jqftu::deck_foundations */
} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
