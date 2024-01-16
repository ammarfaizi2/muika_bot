// SPDX-License-Identifier: GPL-2.0-only

#include "Jqftu.hpp"
#include <cassert>
#include <cctype>
#include <curl/curl.h>

namespace muika {

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
	if (!tmp)
		return 0;

	cr->data = tmp;
	memcpy(cr->data + cr->size, data, real_size);
	cr->size += real_size;
	cr->data[cr->size] = '\0';
	return real_size;
}

static char *generate_latex_image(const char *kanji)
{
	static const char data_template[] = "{\"content\":\"\\\\documentclass[32pt]{article}\\n\\\\usepackage{CJKutf8}\\n\\\\thispagestyle{empty}\\n\\\\begin{document}\\n\\\\begin{CJK}{UTF8}{min}\\n%s\\n\\\\end{CJK}\\n\\\\end{document}\\n\",\"d\":800,\"border\":\"100x80\",\"bcolor\":\"white\"}";
	struct curl_slist *headers = NULL;
	struct curl_resp cr;
	size_t body_len;
	char *req_body;
	CURLcode res;
	CURL *curl;

	body_len = strlen(data_template) + strlen(kanji);
	req_body = (char *)malloc(body_len + 1);
	if (!req_body)
		return NULL;

	curl = curl_easy_init();
	if (!curl) {
		free(req_body);
		return NULL;
	}

	cr.data = NULL;
	cr.size = 0;
	snprintf(req_body, body_len + 1, data_template, kanji);

	headers = curl_slist_append(headers, "Content-Type: text/plain;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_URL, "https://latex.teainside.org/api.php?action=tex2png_no_op");
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_body);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(req_body));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cr);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	free(req_body);

	if (res != CURLE_OK) {
		free(cr.data);
		return NULL;
	}

	return cr.data;
}

static char *generate_text_image(const char *kanji)
{
	static const char url_template[] = "https://latex.teainside.org/api.php?action=file&type=png&hash=";
	char *json_str = generate_latex_image(kanji);
	size_t len;
	char *ret;
	json j;

	if (!json_str)
		return NULL;

	try {
		j = json::parse(json_str);
	} catch (json::parse_error &e) {
		free(json_str);
		return NULL;
	}

	if (!j.is_object())
		goto out_err;

	if (j.find("res") == j.end())
		goto out_err;

	if (!j["res"].is_string())
		goto out_err;

	len = strlen(url_template) + j["res"].get<std::string>().length();
	ret = (char *)malloc(len + 1);
	if (!ret)
		goto out_err;

	snprintf(ret, len + 1, "%s%s", url_template, j["res"].get<std::string>().c_str());
	free(json_str);
	return ret;

out_err:
	free(json_str);
	return NULL;
}

Jqftu::Jqftu(Muika &m, TgBot::Message::Ptr &msg):
	m_(m),
	msg_(msg)
{
}

Jqftu::~Jqftu(void)
{
}

inline void Jqftu::parseArgs(void)
{
	const char *start = msg_->text.c_str() + 6;
	const char *end = msg_->text.c_str() + msg_->text.length();

	while (start < end) {
		while (start < end && std::isspace(*start))
			start++;

		if (start == end)
			break;

		const char *arg_start = start;
		while (start < end && !std::isspace(*start))
			start++;

		args_.push_back(std::string(arg_start, start - arg_start));
	}
}

static const std::string help_str =
	"Say <code>/jqftu deckname</code> to start a quiz (Example: <code>/jqftu tozai_line</code>).\n\n"
	"List Japanese Station Name Decks:\n"
	"- (T) <code>tozai_line</code> - Tokyo Metro Tozai Line (東西線)\n"
	"- (KK) <code>keikyu_line</code> - Keikyu Main Line (京急本線)\n"
	"- (KS) <code>keisei_line</code> - Keisei Main Line (京成本線)\n"
	"- (JY) <code>yamanote_line</code> - JR Yamanote Line (山手線)\n"
	"- (JB) <code>chuo_sobu_line</code> - JR Chuo-Sobu Line (中央・総武緩行線)\n"
	"- (JC) <code>chuo_line</code> - JR Chuo Line (中央線快速)\n"
	"- (JK) <code>keihin_tohoku_line</code> - JR Keihin-Tohoku Line (京浜東北線)\n"
	"- (JA) <code>saikyo_line</code> - JR Saikyo Line (埼京線)";

inline void Jqftu::showHelp(void)
{
	m_.getBot().getApi().sendMessage(
		msg_->chat->id,
		help_str,
		true,
		msg_->messageId,
		nullptr,
		"HTML"
	);
}

int Jqftu::execute(void)
{
	assert(msg_->text.length() >= 6);
	parseArgs();

	if (args_.empty()) {
		showHelp();
		return 0;
	}

	startJqftu();
	return 0;
}

inline char *Jqftu::getDeckJsonFilePath(void)
{
	static const char file_base_path[] = "./storage/jqftu/decks/";
	char *file_path;
	size_t len;

	/*
	 * Don't allow '.' and '/' in the file name.
	 */
	for (auto &c : args_[0]) {
		if (c == '.' || c == '/')
			return NULL;
	}

	len = strlen(file_base_path) + args_[0].length() + 6;
	file_path = (char *)malloc(len);
	if (!file_path)
		return NULL;

	snprintf(file_path, len, "%s%s.json", file_base_path, args_[0].c_str());
	printf("file_path: %s\n", file_path);
	return file_path;
}

inline char *Jqftu::getDeckJsonString(void)
{
	char *file_content;
	size_t file_size;
	char *file_path;
	FILE *fp;

	file_path = getDeckJsonFilePath();
	if (!file_path)
		return NULL;

	fp = fopen(file_path, "rb");
	free(file_path);
	if (!fp)
		return NULL;

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	file_content = (char *)malloc(file_size + 1);
	if (!file_content) {
		fclose(fp);
		return NULL;
	}

	if (fread(file_content, 1, file_size, fp) != file_size) {
		fclose(fp);
		free(file_content);
		return NULL;
	}

	file_content[file_size] = '\0';
	fclose(fp);
	return file_content;
}

inline int Jqftu::randomPickFromDeck(std::string &kanji, std::string &romaji,
				     std::string &hiragana, std::string &katakana,
				     std::string &extra)
{
	char *json_str = getDeckJsonString();
	json j;

	if (!json_str)
		return -1;

	try {
		j = json::parse(json_str);
	} catch (json::parse_error &e) {
		free(json_str);
		return -1;
	}

	if (!j.is_array()) {
		free(json_str);
		return -1;
	}

	if (j.size() == 0) {
		free(json_str);
		return -1;
	}

	int index = rand() % j.size();
	kanji = j[index]["kanji"].get<std::string>();
	romaji = j[index]["romaji"].get<std::string>();
	hiragana = j[index]["hiragana"].get<std::string>();
	katakana = j[index]["katakana"].get<std::string>();
	extra = j[index]["n"].get<std::string>();

	free(json_str);
	return 0;
}

static char *generate_text_image(const char *kanji);

inline void Jqftu::startJqftu(void)
{
	std::string kanji, romaji, hiragana, katakana, extra;
	int ret;

	ret = randomPickFromDeck(kanji, romaji, hiragana, katakana, extra);
	if (ret < 0) {
		m_.getBot().getApi().sendMessage(
			msg_->chat->id,
			"Failed to pick a card from the deck.",
			true,
			msg_->messageId
		);
		return;
	}

	char *image = generate_text_image(kanji.c_str());
	if (!image) {
		m_.getBot().getApi().sendMessage(
			msg_->chat->id,
			"Failed to generate the image.",
			true,
			msg_->messageId
		);
		return;
	}

	// Send image (in the URL form)
	m_.getBot().getApi().sendPhoto(
		msg_->chat->id,
		image,
		"",
		msg_->messageId
	);
}


} /* namespace muika */
