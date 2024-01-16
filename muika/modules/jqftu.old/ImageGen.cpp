
#include "ImageGen.hpp"

#include <cctype>
#include <cerrno>
#include <cassert>
#include <cstring>
#include <curl/curl.h>

namespace muika {
namespace modules {
namespace jqftu {

static const char payload_template[] = "{\"content\":\"\\\\documentclass[32pt]{article}\\n\\\\usepackage{CJKutf8}\\n\\\\thispagestyle{empty}\\n\\\\begin{document}\\n\\\\begin{CJK}{UTF8}{min}\\n%s\\n\\\\end{CJK}\\n\\\\end{document}\\n\",\"d\":800,\"border\":\"100x80\",\"bcolor\":\"white\"}";

struct curl_resp {
	char	*data;
	size_t	size;
};

static size_t curl_cb(void *data, size_t size, size_t nmemb, void *userp)
{
	struct curl_resp *cr = (struct curl_resp *)userp;
	size_t real_size = size * nmemb;
	char *tmp;
	
	tmp = (char *)realloc(cr->data, cr->size + real_size + 1);
	if (!tmp) {
		free(cr->data);
		cr->data = NULL;
		return 0;
	}

	cr->data = tmp;
	memcpy(cr->data + cr->size, data, real_size);
	cr->size += real_size;
	cr->data[cr->size] = '\0';
	return real_size;
}

ImageGen::ImageGen(const std::string &text):
	text_(text)
{
}

ImageGen::~ImageGen(void)
{
}

/*
 * Fill this->hash_ if success.
 */
inline int ImageGen::generateImage(void)
{
	struct curl_resp cr;
	size_t payload_len;
	char *payload;
	CURL *curl;
	int ret;

	payload_len = strlen(payload_template) + text_.length();
	payload = (char *)malloc(payload_len + 1);
	if (!payload)
		return -ENOMEM;

	curl = curl_easy_init();
	if (!curl) {
		free(payload);
		return -ENOMEM;
	}

	cr.data = NULL;
	cr.size = 0;
	ret = snprintf(payload, payload_len + 1, payload_template, text_.c_str());

	curl_easy_setopt(curl, CURLOPT_URL, "https://latex.teainside.org/api.php?action=tex2png_no_op");
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, ret);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cr);
	ret = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	free(payload);

	if (ret != CURLE_OK) {
		free(cr.data);
		return -EIO;
	}

}

int ImageGen::generate(void)
{

}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
