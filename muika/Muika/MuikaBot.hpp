// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA__MUIKABOT_HPP
#define MUIKA__MUIKA__MUIKABOT_HPP

#include <memory>

namespace TgBot {
class Bot;
class HttpClient;
} /* namespace TgBot */

#define pr_info(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define pr_err(fmt, ...)  printf("[ERR ] " fmt "\n", ##__VA_ARGS__)
#define pr_warn(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__)

#ifndef __json
#define __json
#endif

namespace Muika {

class ModuleManager;
class MuikaBot {
public:
	explicit MuikaBot(const char *token) noexcept;
	~MuikaBot(void);
	void run(void);
	TgBot::Bot *getBot(void) { return bot_.get(); }
	ModuleManager *getModuleManager(void) { return mod_mgr_.get(); }

private:
	char token_[128];
	std::unique_ptr<TgBot::HttpClient> http_client_;
	std::unique_ptr<TgBot::Bot> bot_;
	std::unique_ptr<ModuleManager> mod_mgr_;

	void installHandlers(void);
};

} /* namespace Muika */

#endif
