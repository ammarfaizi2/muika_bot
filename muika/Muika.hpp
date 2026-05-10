// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA_HPP
#define MUIKA__MUIKA_HPP

#include <muika/Message.hpp>
#include <muika/Module.hpp>
#include <muika/Reactor.hpp>

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <shared_mutex>

namespace muika {

struct MuikaConfig {
	std::string storage_path;
};

typedef std::shared_ptr<Message> MsgPtr;

class Muika {
public:
	Muika(const MuikaConfig &cfg, std::shared_ptr<Reactor> reactor);
	~Muika(void);
	void passMsg(MsgPtr msg);
	void loadModule(const std::string &name);
	void unloadModule(const std::string &name);
	void unloadAllModules(void);
	void disableModule(const std::string &name);
	void enableModule(const std::string &name);
	Module *getModule(const std::string &name);
	Reactor *reactor(void);

	static
	User createUser(const std::string &id,
			const std::string &name,
			const std::string &uname);
	static
	MsgPtr createMsgText(const std::string &chat_id,
			     const std::string &msg_id,
			     const User &user,
			     const std::string &text);

	static
	MsgPtr createMsgText(const std::string &chat_id,
			     const std::string &msg_id,
			     const std::string &user_id,
			     const std::string &name,
			     const std::string &uname,
			     const std::string &text);

private:
	std::shared_ptr<Reactor> reactor_;
	std::vector<std::unique_ptr<Module>> modules_;
	MuikaConfig cfg_;
	std::shared_mutex modules_lock_;
	FILE *lock_file_ = nullptr;
	size_t findModuleIdx(const std::string &name);
};

} /* namespace muika */

#endif /* #ifndef MUIKA__MUIKA_HPP */
