// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/User.hpp>

namespace muika {
namespace modules {
namespace jqftu {

using json = nlohmann::json;

// static
User User::deserialize(const std::string &str)
{
	json j = json::parse(str);

	return User::fromJson(j);
}

// static
User User::fromJson(const json &j)
{
	return User(j["chat_id"], j["id"], j["first_name"], j["last_name"],
		    j["username"], j["point"]);
}

// static
std::string User::serialize(const User &user)
{
	json j = User::toJson(user);

	return j.dump(4, ' ', false);
}

// static
json User::toJson(const User &user)
{
	json j;

	j["chat_id"] = user.chat_id();
	j["id"] = user.id();
	j["first_name"] = user.first_name();
	j["last_name"] = user.last_name();
	j["username"] = user.username();
	j["point"] = user.point();

	return j;
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
