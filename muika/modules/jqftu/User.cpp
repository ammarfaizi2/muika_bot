// SPDX-License-Identifier: GPL-2.0-only

#include <muika/modules/jqftu/User.hpp>

namespace muika {
namespace modules {
namespace jqftu {

using json = nlohmann::json;

json User::toJson(void) const
{
	json j;
	j["chat_id"] = chat_id_;
	j["id"] = id_;
	j["first_name"] = first_name_;
	j["last_name"] = last_name_;
	j["username"] = username_;
	j["point"] = point_;
	return j;
}

std::string User::serialize(void) const
{
	return serialize(*this);
}

// static
json User::toJson(const User &user)
{
	return user.toJson();
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
	json j = user.toJson();
	return j.dump(4, ' ', false);
}

// static
User User::deserialize(const std::string &str)
{
	json j = json::parse(str);
	return User::fromJson(j);
}

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */
