// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Card.hpp>
#include <muika/modules/m002_jqftu/internal.hpp>
#include <muika/helpers.hpp>

#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {

using json = nlohmann::json;

struct curl_resp {
	char *data;
	size_t size;
};

static __thread CURL *t_curl = nullptr;

/*
 * Allow up to romaji_fault_tolerance_ % difference for strings
 * longer than 4 characters.  Shorter strings must match exactly.
 */
inline bool Card::cmpAllowSimilarRomaji(const std::string &a, const std::string &b)
{
	if (a.length() <= 4 || b.length() <= 4)
		return a == b;

	std::size_t min_sim = 100 - getRomajiFaultTolerance();
	std::size_t sim = mk_similar_text(a, b);
	pr_debug("a=%s b=%s sim=%zu", a.c_str(), b.c_str(), sim);
	return sim >= min_sim;
}

bool Card::normalizedRomajiCompare(const std::string &a, const std::string &b)
{
	if (a == b)
		return true;

	std::string aa = a, bb = b;
	mk_strtolower(aa);
	mk_strtolower(bb);
	if (cmpAllowSimilarRomaji(aa, bb))
		return true;

	mk_str_replace(aa, "-", "");
	mk_str_replace(bb, "-", "");
	mk_str_replace(aa, " ", "");
	mk_str_replace(bb, " ", "");
	if (cmpAllowSimilarRomaji(aa, bb))
		return true;

	std::string aa_o = aa, bb_o = bb;
	mk_str_replace(aa_o, "\xc5\x8d", "o"); /* ō */
	mk_str_replace(bb_o, "\xc5\x8d", "o");
	if (cmpAllowSimilarRomaji(aa_o, bb_o))
		return true;

	std::string aa_ou = aa, bb_ou = bb;
	mk_str_replace(aa_ou, "\xc5\x8d", "ou");
	mk_str_replace(bb_ou, "\xc5\x8d", "ou");
	if (cmpAllowSimilarRomaji(aa_ou, bb_ou))
		return true;

	std::string aa_u = aa, bb_u = bb;
	mk_str_replace(aa_u, "\xc5\xab", "u"); /* ū */
	mk_str_replace(bb_u, "\xc5\xab", "u");
	if (cmpAllowSimilarRomaji(aa_u, bb_u))
		return true;

	std::string aa_uu = aa, bb_uu = bb;
	mk_str_replace(aa_uu, "\xc5\xab", "uu");
	mk_str_replace(bb_uu, "\xc5\xab", "uu");
	if (cmpAllowSimilarRomaji(aa_uu, bb_uu))
		return true;

	std::string aa_n = aa, bb_n = bb;
	mk_str_replace(aa_n, "n", "nn");
	mk_str_replace(bb_n, "n", "nn");
	if (cmpAllowSimilarRomaji(aa_n, bb_n))
		return true;

	return false;
}

void Card::threadInit(void)
{
	if (!t_curl)
		t_curl = curl_easy_init();
}

void Card::threadFree(void)
{
	if (t_curl) {
		curl_easy_cleanup(t_curl);
		t_curl = nullptr;
	}
}

const std::vector<std::string> &Card::getCardPhotos(void)
{
	static const std::vector<std::string> empty;
	return empty;
}

static size_t curl_cb(void *data, size_t size, size_t nmemb, void *userp)
{
	struct curl_resp *cr = (struct curl_resp *)userp;
	size_t real_size = size * nmemb;
	char *tmp = (char *)realloc(cr->data, cr->size + real_size + 1);
	if (!tmp)
		return 0;

	cr->data = tmp;
	memcpy(cr->data + cr->size, data, real_size);
	cr->size += real_size;
	cr->data[cr->size] = '\0';
	return real_size;
}

// static
std::string Card::generateLatexPng(const char *latex_code)
{
	if (!t_curl)
		threadInit();
	CURL *curl = t_curl;
	if (!curl)
		throw std::runtime_error("generateLatexPng(): curl_easy_init() failed");

	struct curl_resp cr = { nullptr, 0 };
	curl_easy_setopt(curl, CURLOPT_URL, "https://latex.teainside.org/api.php?action=tex2png_no_op");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, latex_code);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(latex_code));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cr);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		free(cr.data);
		throw std::runtime_error("generateLatexPng(): curl_easy_perform() failed: " +
					 std::string(curl_easy_strerror(res)));
	}

	std::string ret = cr.data ? std::string(cr.data) : std::string();
	free(cr.data);
	return ret;
}

// static
std::string Card::generateLatexPngUrl(const char *latex_code)
{
	std::string json_str = generateLatexPng(latex_code);
	json j;

	try {
		j = json::parse(json_str);
	} catch (json::parse_error &) {
		throw std::runtime_error("generateLatexPngUrl(): json::parse() failed");
	}

	if (!j.is_object())
		throw std::runtime_error("generateLatexPngUrl(): json is not an object");

	if (j.find("res") == j.end() || !j["res"].is_string())
		throw std::runtime_error("generateLatexPngUrl(): no string \"res\" key");

	return "https://latex.teainside.org/api.php?action=file&type=png&hash=" +
	       j["res"].get<std::string>();
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
