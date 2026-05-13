// SPDX-License-Identifier: GPL-2.0-only
#include <mtgbot/Bot.hpp>
#include <mlogger/mlogger.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

mk_logger_t *create_logger(void)
{
	const char *path = getenv("MUIKA_TGBOT_LOG_FILE");
	mk_logger_t *l;

	if (path && *path) {
		l = mk_logger_init(path);
		if (!l) {
			fprintf(stderr, "Failed to open log file %s, "
					"falling back to stderr\n", path);
			l = mk_logger_init_file(stderr);
		}
	} else {
		l = mk_logger_init_file(stderr);
	}

	if (!l)
		return nullptr;

	const char *lvl = getenv("MUIKA_TGBOT_LOG_LEVEL");
	if (lvl && *lvl) {
		if      (!strcmp(lvl, "debug")) mk_logger_set_level(l, MK_LOG_DEBUG);
		else if (!strcmp(lvl, "info"))  mk_logger_set_level(l, MK_LOG_INFO);
		else if (!strcmp(lvl, "warn"))  mk_logger_set_level(l, MK_LOG_WARN);
		else if (!strcmp(lvl, "error")) mk_logger_set_level(l, MK_LOG_ERROR);
		else if (!strcmp(lvl, "fatal")) mk_logger_set_level(l, MK_LOG_FATAL);
	} else {
		mk_logger_set_level(l, MK_LOG_INFO);
	}
	return l;
}

std::unique_ptr<mtgbot::Bot> create_mtgbot(mk_logger_t *log)
{
	const char *token = getenv("MUIKA_TGBOT_TOKEN");
	if (!token) {
		mk_fatal(log, "MUIKA_TGBOT_TOKEN environment variable is not set");
		return nullptr;
	}

	mtgbot::BotConfig cfg;
	cfg.token = token;
	cfg.storage_path = "./storage/mtgbot";
	cfg.logger = log;
	return std::make_unique<mtgbot::Bot>(cfg);
}

} /* anonymous namespace */

int main(void)
{
	mk_logger_t *log = create_logger();
	if (!log) {
		fprintf(stderr, "Failed to initialize logger\n");
		return 1;
	}

	int rc = 0;
	{
		std::unique_ptr<mtgbot::Bot> bot = create_mtgbot(log);
		if (!bot)
			rc = 1;
		else
			bot->run();
	}

	mk_logger_free(log);
	return rc;
}
