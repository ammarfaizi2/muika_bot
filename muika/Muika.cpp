// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m001_hello/Module.hpp>
#include <muika/helpers.hpp>
#include <muika/Muika.hpp>

#include <stdexcept>
#include <cstdio>
#include <sys/file.h>

namespace muika {

static void createStorageDir(const std::string &storage_path)
{
	int r = mk_mkdir_p(storage_path.c_str(), 0755);
	if (r < 0)
		throw std::runtime_error("Failed to create storage directory "
					 + storage_path);
}

static FILE *openLockFile(const std::string &storage_path)
{
	std::string lock_path = storage_path + "/lock";
	FILE *f = fopen(lock_path.c_str(), "wb");
	if (!f)
		throw std::runtime_error("Failed to open lock file " + lock_path);

	if (flock(fileno(f), LOCK_EX | LOCK_NB) < 0) {
		fclose(f);
		throw std::runtime_error("Failed to acquire lock on " + lock_path);
	}
	return f;
}

Muika::Muika(const MuikaConfig &cfg, std::shared_ptr<Reactor> reactor):
	reactor_(reactor),
	modules_(),
	cfg_(cfg),
	lock_file_(nullptr)
{
	createStorageDir(cfg_.storage_path);
	lock_file_ = openLockFile(cfg_.storage_path);
}

Muika::~Muika(void)
{
	if (lock_file_) {
		flock(fileno(lock_file_), LOCK_UN);
		fclose(lock_file_);
	}

	unloadAllModules();
}

// static
User Muika::createUser(const std::string &id, const std::string &name,
		       const std::string &uname)
{
	return User(id, name, uname);
}

// static
MsgPtr Muika::createMsgText(const std::string &chat_id,
			    const std::string &id,
			    const User &user,
			    const std::string &text)
{
	MessageContent content(text);
	return std::make_shared<Message>(chat_id, id, user, content);
}

// static
MsgPtr Muika::createMsgText(const std::string &chat_id,
			    const std::string &id,
			    const std::string &user_id,
			    const std::string &name,
			    const std::string &uname,
			    const std::string &text)
{
	User user = createUser(user_id, name, uname);
	return createMsgText(chat_id, id, user, text);
}

void Muika::passMsg(MsgPtr msg)
{
	std::shared_lock<std::shared_mutex> lock(modules_lock_);

	for (const auto &mod : modules_) {
		if (!mod->enabled())
			continue;

		Module::Type r = mod->passMsg(msg);
		if (r == Module::Type::MSG_HANDLED)
			break;
	}
}

#define MODULE_IDX_NOENT ((size_t)-1)

inline
size_t Muika::findModuleIdx(const std::string &name)
{
	for (size_t i = 0; i < modules_.size(); ++i) {
		if (modules_[i]->moduleName() == name)
			return i;
	}
	return MODULE_IDX_NOENT;
}

void Muika::loadModule(const std::string &name)
{
	std::unique_lock<std::shared_mutex> lock(modules_lock_);
	if (findModuleIdx(name) != MODULE_IDX_NOENT)
		return;

	if (name == "hello") {
		auto mod = std::make_unique<modules::m001_hello::Module>();
		mod->setMk(this);
		mod->init();
		modules_.push_back(std::move(mod));
		return;
	}

	throw std::runtime_error("Unknown module: " + name);
}

void Muika::unloadModule(const std::string &name)
{
	std::unique_lock<std::shared_mutex> lock(modules_lock_);
	size_t idx = findModuleIdx(name);
	if (idx == MODULE_IDX_NOENT)
		return;

	modules_[idx]->free();
	modules_.erase(modules_.begin() + idx);
}

void Muika::unloadAllModules(void)
{
	std::unique_lock<std::shared_mutex> lock(modules_lock_);
	for (auto &mod : modules_)
		mod->free();

	modules_.clear();
}

void Muika::disableModule(const std::string &name)
{
	std::unique_lock<std::shared_mutex> lock(modules_lock_);
	size_t idx = findModuleIdx(name);
	if (idx == MODULE_IDX_NOENT)
		return;

	modules_[idx]->setEnabled(false);
}

void Muika::enableModule(const std::string &name)
{
	std::unique_lock<std::shared_mutex> lock(modules_lock_);
	size_t idx = findModuleIdx(name);
	if (idx == MODULE_IDX_NOENT)
		return;

	modules_[idx]->setEnabled(true);
}

Module *Muika::getModule(const std::string &name)
{
	std::shared_lock<std::shared_mutex> lock(modules_lock_);
	size_t idx = findModuleIdx(name);
	if (idx == MODULE_IDX_NOENT)
		return nullptr;

	return modules_[idx].get();
}

Reactor *Muika::reactor(void)
{
	return reactor_.get();
}

} /* namespace muika */
