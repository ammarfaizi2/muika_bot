// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/SessionMap.hpp>

#include <muika/helpers.hpp>

namespace muika {
namespace modules {
namespace jqftu {

SessionMap::SessionMap(Muika &m):
	m_(m)
{
}

SessionMap::~SessionMap(void) = default;

void SessionMap::loadAll(void)
{
	std::string sess_dir = m_.getStorageDir() + "/jqftu/sessions";
	std::vector<std::string> files;

	mkdir_recursive(sess_dir, 0755);
	files = scandir(sess_dir);

	for (const auto &file : files) {
		const char *fn = file.c_str();

		/*
		 * Make sure it ends with .json
		 */
		if (strlen(fn) < 6 || strcmp(fn + strlen(fn) - 5, ".json"))
			continue;

		std::string chat_id_str = std::string(fn, strlen(fn) - 5);
		if (!is_number(chat_id_str))
			continue;

		load(std::stoll(chat_id_str));
	}
}

void SessionMap::load(int64_t chat_id)
{
	pr_debug("jqftu: Loading session for chat_id %ld", chat_id);
}

std::shared_ptr<Session>
SessionMap::create(int64_t chat_id, const std::vector<std::string> &deck_list)
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (sessions_.find(chat_id) != sessions_.end())
		throw std::runtime_error("Session already exists!");

	pr_debug("jqftu: Creating session for chat_id %ld", chat_id);

	std::shared_ptr<Session> sess
		= std::make_shared<Session>(m_, chat_id, deck_list);

	sessions_[chat_id] = sess;
	return sess;
}

void SessionMap::save(void)
{
	for (const auto &it : sessions_) {
		pr_debug("jqftu: Saving session for chat_id %ld", it.first);
	}
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
