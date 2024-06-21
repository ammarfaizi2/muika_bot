// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA_HPP
#define MUIKA__MUIKA_HPP

#include <tgbot/tgbot.h>
#include <muika/helpers.hpp>

#include <string>
#include <memory>

namespace muika {

class ModuleManager;

class Muika {
private:
	TgBot::Bot bot_;
	std::string storage_dir_;
	std::unique_ptr<ModuleManager> mod_mgr_;

	void handleMessage(TgBot::Message::Ptr &msg);

public:
	Muika(const std::string &token, const std::string &storage_dir);
	~Muika(void);

	void start(void);

	inline auto &getApi(void) { return bot_.getApi(); }
	inline TgBot::Bot &getBot(void) { return bot_; }
	inline const std::string &getStorageDir(void) const { return storage_dir_; }
};

} /* namespace muika */

#endif /* #ifndef MUIKA__MUIKA_HPP */
