// SPDX-License-Identifier: GPL-2.0-only

#include <muika/Muika/Modules/Jqftu/Session.hpp>
#include <muika/helpers.hpp>

#include <cassert>
#include <cstdio>
#include <nlohmann/json.hpp>

namespace Muika {
namespace Modules {
namespace Jqftu {

Session::Session(ModJqftu *mj):
	mj_(mj)
{
}

Session::~Session(void)
{
}

bool Session::saveToFile(const std::string &path)
{
	nlohmann::json j;

	j["chat_id"] = chat_id_;
	j["chat_title"] = chat_title_;
	j["sess_last_msg_id"] = sess_last_msg_id_;

	std::string sjson = j.dump(2);
	int err = file_put_contents(path, sjson);
	if (err) {
		pr_err("Jqftu: Failed to write session file '%s': %s",
			path.c_str(), strerror(-err));
		return false;
	}

	return true;
}

std::unique_ptr<Session> Session::loadFromFile(const std::string &path)
{
	std::string sjson;
	int err;

	err = file_get_contents(path, sjson);
	if (err) {
		pr_err("Jqftu: Failed to read session file '%s': %s",
			path.c_str(), strerror(-err));
		return nullptr;
	}

	try {
		auto j = nlohmann::json::parse(sjson);
		std::unique_ptr<Session> sess = std::make_unique<Session>(nullptr);
		sess->initData(j.at("chat_id").get<int64_t>(),
				j.at("chat_title").get<std::string>(),
				j.at("sess_last_msg_id").get<uint64_t>());
		return sess;
	} catch (const std::exception &e) {
		pr_err("Jqftu: Failed to parse session file '%s': %s",
			path.c_str(), e.what());
		return nullptr;
	}
}

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */
