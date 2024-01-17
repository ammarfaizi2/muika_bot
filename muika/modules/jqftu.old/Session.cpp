// SPDX-License-Identifier: GPL-2.0-only

#include "Session.hpp"
#include "internal.hpp"
#include "entry.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using TgBot::Message;

namespace muika {
namespace modules {
namespace jqftu {

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

static char *generate_latex_text(const char *kanji)
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
	char *json_str = generate_latex_text(kanji);
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

	if (!j.is_object()) {
		free(json_str);
		return NULL;
	}

	if (j.find("res") == j.end()) {
		free(json_str);
		return NULL;
	}

	if (!j["res"].is_string()) {
		free(json_str);
		return NULL;
	}

	len = strlen(url_template) + j["res"].get<std::string>().length();
	ret = (char *)malloc(len + 1);
	if (!ret) {
		free(json_str);
		return NULL;
	}

	snprintf(ret, len + 1, "%s%s", url_template, j["res"].get<std::string>().c_str());
	free(json_str);
	return ret;
}

Session::Session(Muika &m, int64_t chat_id, const std::string &deck_name):
	m_(m),
	chat_id_(chat_id),
	deck_(deck_name)
{
}

inline void Session::destroySession(void)
{
	std::unique_lock<std::mutex> lock(sessions_mutex);
	sessions.erase(chat_id_);
}

Session::~Session(void)
{
	destroySession();
}

void Session::drawAndSendCardLocked(void)
{
	Message::Ptr msg_p;
	Card *c = deck_.drawCard();
	char *img_url;

	if (!c) {
		m_.getBot().getApi().sendMessage(
			chat_id_,
			"Quiz finished!",
			true
		);
		delete this;
		return;
	}

	img_url = generate_text_image(c->kanji.c_str());
	if (!img_url) {
		m_.getBot().getApi().sendMessage(
			chat_id_,
			"Failed to generate image.",
			true
		);
		delete this;
		return;
	}

	pr_debug("Current card: %s\n", c->romaji.c_str());
	msg_p = m_.getBot().getApi().sendPhoto(chat_id_, img_url,
			"Scope: Tokyo Metro Tozai Line (東西線)\n\n"
			"Write the kana or romaji of this kanji!\n"
			"Time limit: 30 minutes");
	free(img_url);
	current_card_ = c;
	last_msg_id_ = msg_p->messageId;
}

void Session::drawAndSendCard(void)
{
	std::unique_lock<std::mutex> lock(mutex_);
	drawAndSendCardLocked();
}

void Session::worker(void)
{
	pr_debug("Worker started.\n");

	std::unique_lock<std::mutex> lock(mutex_);
	while (!deck_.isFinished()) {
		drawAndSendCardLocked();
		cv_.wait_for(lock, std::chrono::seconds(1200));
		if (!current_card_)
			continue;

		m_.getBot().getApi().sendMessage(
			chat_id_,
			"Time's up!\n\n"
			"Q: " + current_card_->kanji + "\n"
			"Station Number: " + current_card_->extra + "\n"
			"The answer is: " + current_card_->hiragana + " (" + current_card_->romaji + ")",
			true,
			last_msg_id_
		);
		current_card_ = nullptr;
	}

	m_.getBot().getApi().sendMessage(
		chat_id_,
		"Quiz finished!",
		true
	);

	lock.unlock();
	delete this;
}

int Session::start(void)
{
	if (deck_.loadDeck() < 0)
		return -1;

	try {
		worker_ = std::thread(&Session::worker, this);
		worker_.detach();
	} catch (std::system_error &e) {
		return -1;
	}

	return 0;
}

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

void Session::handleCorrectAnswer(TgBot::Message::Ptr &msg)
{
	m_.getBot().getApi().sendMessage(
		chat_id_,
		"Correct!\n\n"
		"Q: " + current_card_->kanji + "\n"
		"Station Number: " + current_card_->extra + "\n"
		"The answer is: " + current_card_->hiragana + " (" + current_card_->romaji + ")",
		true,
		msg->messageId
	);
	current_card_ = nullptr;
	cv_.notify_one();
}

void Session::handleAnswer(TgBot::Message::Ptr &msg)
{
	std::unique_lock<std::mutex> lock(mutex_);
	std::string uanswer, canswer;

	if (!current_card_)
		return;

	uanswer = msg->text;
	if (uanswer == current_card_->hiragana) {
		handleCorrectAnswer(msg);
		return;
	}

	canswer = current_card_->romaji;
	str_replace(canswer, "-", " ");
	str_replace(canswer, " ", "");
	strtolower(canswer);

	str_replace(uanswer, "-", " ");
	str_replace(uanswer, " ", "");
	strtolower(uanswer);

	if (uanswer == canswer) {
		handleCorrectAnswer(msg);
		return;
	}

	std::string o_to_o = canswer;
	str_replace(o_to_o, "ō", "o");
	if (o_to_o == uanswer) {
		handleCorrectAnswer(msg);
		return;
	}

	std::string o_to_ou = canswer;
	str_replace(o_to_ou, "ō", "ou");
	if (o_to_ou == uanswer) {
		handleCorrectAnswer(msg);
		return;
	}
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
