
#include "entry.hpp"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#if 1
#define pr_debug(fmt, ...) printf("jqftu: " fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) do {} while (0)
#endif

namespace muika {
namespace modules {
namespace jqftu {

struct Card {
	std::string kanji;
	std::string romaji;
	std::string hiragana;
	std::string katakana;
	std::string extra;
};

class Deck {
private:
	std::string name_;
	std::vector<Card> cards_;
	char *getDeckFilePath(void);
	char *getDeckJsonString(void);
public:
	Deck(std::string name);
	~Deck(void) = default;
	int loadDeck(void);
};

inline Deck::Deck(std::string name):
	name_(name)
{
}

inline char *Deck::getDeckFilePath(void)
{
	static const char file_base_path[] = "./storage/jqftu/decks/";
	char *file_path;
	size_t len;

	/*
	 * Don't allow '.' and '/' in the file name.
	 */
	for (auto &c : name_) {
		if (c == '.' || c == '/')
			return NULL;
	}

	len = strlen(file_base_path) + name_.length() + 6;
	file_path = (char *)malloc(len);
	if (!file_path)
		return NULL;

	snprintf(file_path, len, "%s%s.json", file_base_path, name_.c_str());
	pr_debug("Deck file path: %s\n", file_path);
	return file_path;
}

inline char *Deck::getDeckJsonString(void)
{
	char *json_str;
	long file_size;
	char *path;
	FILE *fp;

	path = getDeckFilePath();
	if (!path)
		return NULL;

	fp = fopen(path, "rb");
	free(path);
	if (!fp) {
		pr_debug("Failed to open deck file: %s: %s\n", strerror(errno), path);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	json_str = (char *)malloc(file_size + 1);
	if (!json_str) {
		fclose(fp);
		pr_debug("Failed to allocate memory for deck file: %s\n", path);
		return NULL;
	}

	if (fread(json_str, 1, file_size, fp) != (size_t)file_size) {
		fclose(fp);
		free(json_str);
		pr_debug("Failed to read deck file: %s\n", path);
		return NULL;
	}

	json_str[file_size] = '\0';
	return json_str;
}

inline int Deck::loadDeck(void)
{
	char *json_str;
	json j;

	json_str = getDeckJsonString();
	if (!json_str)
		return -1;

	try {
		j = json::parse(json_str);
	} catch (json::parse_error &e) {
		pr_debug("Failed to parse deck file: %s\n", e.what());
		free(json_str);
		return -1;
	}

	if (!j.is_array()) {
		pr_debug("Deck file is not an array.\n");
		free(json_str);
		return -1;
	}

	for (auto &card : j) {
		Card c;

		if (!card.is_object()) {
			pr_debug("Card is not an object.\n");
			free(json_str);
			return -1;
		}

		if (!card["kanji"].is_string() ||
		    !card["romaji"].is_string() ||
		    !card["hiragana"].is_string() ||
		    !card["katakana"].is_string() ||
		    !card["extra"].is_string()) {
			pr_debug("Card field is not a string.\n");
			free(json_str);
			return -1;
		}

		c.kanji = card["kanji"].get<std::string>();
		c.romaji = card["romaji"].get<std::string>();
		c.hiragana = card["hiragana"].get<std::string>();
		c.katakana = card["katakana"].get<std::string>();
		c.extra = card["extra"].get<std::string>();
		cards_.push_back(c);
	}

	free(json_str);
	return 0;
}

class Session {
private:
	Muika &m_;
	int64_t chat_id_;
	std::mutex mutex_;
	std::thread worker_;
	Deck deck_;
	void worker(void);
public:
	Session(Muika &m, int64_t chat_id, const std::string &deck_name);
	~Session(void);
	int start(void);
	inline int64_t getChatId(void) const { return chat_id_; }
};

static std::unordered_map<int64_t, Session *> sessions;
static std::mutex sessions_mutex;

inline Session::Session(Muika &m, int64_t chat_id, const std::string &deck_name):
	m_(m),
	chat_id_(chat_id),
	deck_(deck_name)
{
}

inline Session::~Session(void)
{
}

inline void Session::worker(void)
{
}

inline int Session::start(void)
{
	try {
		worker_ = std::thread(&Session::worker, this);
	} catch (std::system_error &e) {
		return -1;
	}

	return 0;
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

class Command {
private:
	Muika &m_;
	TgBot::Message::Ptr &msg_;
	std::vector<std::string> args_;
	void parseArgs(void);
	void showHelp(void);
	void start(void);

public:
	Command(Muika &m, TgBot::Message::Ptr &msg);
	~Command(void) = default;
	void execute(void);
};

inline Command::Command(Muika &m, TgBot::Message::Ptr &msg):
	m_(m),
	msg_(msg)
{
}

inline void Command::parseArgs(void)
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

inline void Command::showHelp(void)
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

inline void Command::start(void)
{
	std::unique_lock<std::mutex> lock(sessions_mutex);
	Session *sess;

	if (sessions.find(msg_->chat->id) == sessions.end()) {
		sess = new Session(m_, msg_->chat->id, args_[0]);
		sessions[msg_->chat->id] = sess;
		sess->start();
	} else {
		lock.unlock();
		m_.getBot().getApi().sendMessage(
			msg_->chat->id,
			"Another quiz is running. Please finish it first.",
			true,
			msg_->messageId
		);
	}
}

inline void Command::execute(void)
{
	assert(msg_->text.length() >= 6);
	parseArgs();

	if (args_.size() == 0) {
		showHelp();
		return;
	}

	start();
}

static bool is_space_or_null(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\0';
}

static module_ret_t handle_jqftu_command(muika::Muika &m,
					 TgBot::Message::Ptr &msg)
{
	const char *txt = msg->text.c_str();

	if (msg->text.length() < 6)
		return MOD_ENTRY_CONTINUE;

	/*
	 * The command must start with '/' or '!' or '.'.
	 */
	if (txt[0] != '/' && txt[0] != '!' && txt[0] != '.')
		return MOD_ENTRY_CONTINUE;

	if (memcmp(txt + 1, "jqftu", 5))
		return MOD_ENTRY_CONTINUE;

	if (!is_space_or_null(txt[6]))
		return MOD_ENTRY_CONTINUE;

	Command c(m, msg);
	c.execute();
	return MOD_ENTRY_STOP;
}

module_ret_t entry(muika::Muika &m, TgBot::Message::Ptr &msg)
{
	return handle_jqftu_command(m, msg);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
