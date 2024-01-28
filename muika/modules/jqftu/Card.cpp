// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/internal.hpp"
#include "muika/modules/jqftu/Card.hpp"
#include "muika/helpers.hpp"

#include <mutex>
#include <string>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using muika::str_replace;
using muika::strtolower;
using muika::similar_text;

namespace muika {
namespace modules {
namespace jqftu {

struct curl_resp {
	char	*data;
	size_t	size;
};

static __thread CURL *t_curl = NULL;

#if 0
#define LOCAL_DBG(fmt, ...) printf("jqftu::Card: " fmt "\n", ##__VA_ARGS__)
#else
#define LOCAL_DBG(fmt, ...) do {} while (0)
#endif

/*
 * Allow 90% similarity for strings longer than 4 characters.
 */
inline bool Card::cmpAllowSimilarRomaji(const std::string &a, const std::string &b)
{
	size_t min_sim = 100 - getRomajiFaultTolerance();
	size_t sim;

	if (a.length() <= 4 || b.length() <= 4)
		return a == b;

	sim = similar_text(a, b);
	LOCAL_DBG("a: %s", a.c_str());
	LOCAL_DBG("b: %s", b.c_str());
	LOCAL_DBG("sim: %zu", sim);
	if (sim >= min_sim)
		return true;

	return false;
}

bool Card::normalizedRomajiCompare(const std::string &a, const std::string &b)
{
	if (a == b)
		return true;

	std::string aa = a, bb = b;

	strtolower(aa);
	strtolower(bb);
	if (cmpAllowSimilarRomaji(aa, bb))
		return true;

	str_replace(aa, "-", "");
	str_replace(bb, "-", "");
	str_replace(aa, " ", "");
	str_replace(bb, " ", "");
	if (cmpAllowSimilarRomaji(aa, bb))
		return true;

	std::string aa_o = aa, bb_o = bb;
	str_replace(aa_o, "ō", "o");
	str_replace(bb_o, "ō", "o");
	if (cmpAllowSimilarRomaji(aa_o, bb_o))
		return true;

	std::string aa_ou = aa, bb_ou = bb;
	str_replace(aa_o, "ō", "ou");
	str_replace(bb_o, "ō", "ou");
	if (cmpAllowSimilarRomaji(aa_ou, bb_ou))
		return true;

	std::string aa_u = aa, bb_u = bb;
	str_replace(aa_u, "ū", "u");
	str_replace(bb_u, "ū", "u");
	if (cmpAllowSimilarRomaji(aa_u, bb_u))
		return true;

	std::string aa_uu = aa, bb_uu = bb;
	str_replace(aa_uu, "ū", "uu");
	str_replace(bb_uu, "ū", "uu");
	if (cmpAllowSimilarRomaji(aa_uu, bb_uu))
		return true;

	std::string aa_n = aa, bb_n = bb;
	str_replace(aa_n, "n", "nn");
	str_replace(bb_n, "n", "nn");
	if (cmpAllowSimilarRomaji(aa_n, bb_n))
		return true;

	LOCAL_DBG("a: %s", aa.c_str());
	LOCAL_DBG("b: %s", bb.c_str());
	LOCAL_DBG("a_o: %s", aa_o.c_str());
	LOCAL_DBG("b_o: %s", bb_o.c_str());
	LOCAL_DBG("a_ou: %s", aa_ou.c_str());
	LOCAL_DBG("b_ou: %s", bb_ou.c_str());
	LOCAL_DBG("a_u: %s", aa_u.c_str());
	LOCAL_DBG("b_u: %s", bb_u.c_str());
	LOCAL_DBG("a_uu: %s", aa_uu.c_str());
	LOCAL_DBG("b_uu: %s", bb_uu.c_str());
	LOCAL_DBG("a_n: %s", aa_n.c_str());
	LOCAL_DBG("b_n: %s", bb_n.c_str());
	return false;
}

void Card::threadInit(void)
{
	t_curl = curl_easy_init();
}

void Card::threadFree(void)
{
	if (t_curl) {
		curl_easy_cleanup(t_curl);
		t_curl = NULL;
	}
}

static size_t curl_cb(void *data, size_t size, size_t nmemb, void *userp)
{
	struct curl_resp *cr = (struct curl_resp *)userp;
	size_t real_size = size * nmemb;
	char *tmp;

	tmp = (char *)realloc(cr->data, cr->size + real_size + 1);
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
	struct curl_resp cr;
	CURLcode res;
	CURL *curl;

	curl = t_curl;
	if (!curl) {
		threadInit();
		curl = t_curl;
		if (!curl)
			throw std::runtime_error("generateLatexPng(): curl_easy_init() failed");
	}

	cr.data = NULL;
	cr.size = 0;

	curl_easy_setopt(curl, CURLOPT_URL, "https://latex.teainside.org/api.php?action=tex2png_no_op");
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, latex_code);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(latex_code));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cr);
	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		free(cr.data);
		throw std::runtime_error("generateLatexPng(): curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
	}

	try {
		std::string ret(cr.data);
		free(cr.data);
		return ret;
	} catch (std::exception &e) {
		free(cr.data);
		throw std::runtime_error("generateLatexPng(): std::string() failed");
	}
}

// static
std::string Card::generateLatexPngUrl(const char *latex_code)
{
	std::string hash, json_str;
	json j;

	json_str = generateLatexPng(latex_code);

	try {
		j = json::parse(json_str);
	} catch (json::parse_error &e) {
		throw std::runtime_error("generateLatexPngUrl(): json::parse() failed");
	}

	if (!j.is_object())
		throw std::runtime_error("generateLatexPngUrl(): json is not an object");

	if (j.find("res") == j.end() || !j["res"].is_string())
		throw std::runtime_error("generateLatexPngUrl(): json does not contain \"res\" or \"res\" is not a string");

	return "https://latex.teainside.org/api.php?action=file&type=png&hash=" + j["res"].get<std::string>();
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
