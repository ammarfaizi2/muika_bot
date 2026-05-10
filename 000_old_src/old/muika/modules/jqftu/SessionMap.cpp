// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/SessionMap.hpp"
#include "muika/modules/jqftu/internal.hpp"

#include <cstdio>
#include <dirent.h>

namespace muika {
namespace modules {
namespace jqftu {

std::shared_ptr<Session> SessionMap::__find(int64_t chat_id)
	__must_hold(&mutex_)
{
	auto it = sessions_.find(chat_id);
	if (it == sessions_.end())
		return nullptr;

	return it->second;
}

bool SessionMap::__remove(int64_t chat_id)
	__must_hold(&mutex_)
{
	auto it = sessions_.find(chat_id);
	if (it == sessions_.end())
		return false;

	sessions_.erase(it);
	return true;
}

void SessionMap::__insert(int64_t chat_id, std::shared_ptr<Session> session)
	__must_hold(&mutex_)
{
	sessions_.insert(std::make_pair(chat_id, session));
}

std::shared_ptr<Session> SessionMap::find(int64_t chat_id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	return __find(chat_id);
}

bool SessionMap::remove(int64_t chat_id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	return __remove(chat_id);
}

void SessionMap::insert(int64_t chat_id, std::shared_ptr<Session> session)
{
	std::lock_guard<std::mutex> lock(mutex_);
	__insert(chat_id, session);
}

void SessionMap::loadSessionsFromDisk(Muika &m)
{
	std::lock_guard<std::mutex> lock(mutex_);
	char path[4096];
	DIR *dir;

	snprintf(path, sizeof(path), "./storage/jqftu/sessions");
	dir = opendir(path);
	if (!dir)
		return;

	while (true) {
		struct dirent *ent = readdir(dir);
		size_t len;
		FILE *fp;

		if (!ent)
			break;

		len = strlen(ent->d_name);
		if (len < 5 || strcmp(ent->d_name + len - 5, ".json"))
			continue;

		snprintf(path, sizeof(path), "./storage/jqftu/sessions/%s", ent->d_name);
		fp = fopen(path, "rb");
		if (!fp) {
			pr_debug("Failed to open session file %s: %s\n", path, strerror(errno));
			continue;
		}

		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		std::shared_ptr<Session> sess;

		try {
			std::string json_str(len, '\0');
			if (fread(&json_str[0], 1, len, fp) != len)
				throw std::runtime_error("Failed to read session file: fread() returned unexpected value");

			sess = Session::fromJsonString(m, json_str);
			fclose(fp);
		} catch (const std::exception &e) {
			fclose(fp);
			pr_debug("Failed to load session file %s: %s", path, e.what());
			continue;
		}

		try {
			sess->sendRebootMessage();
			sess->start(true);
			sess->giveSelfPtr(&sess);
		} catch (const std::exception &e) {
			sess->stop();
			pr_debug("Failed to start session %s: %s", path, e.what());
			continue;
		}

		try {
			__insert(sess->getChatId(), sess);
		} catch (const std::exception &e) {
			sess->stop();
			pr_debug("Failed to insert session %s: %s", path, e.what());
		}
	}

	closedir(dir);
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
