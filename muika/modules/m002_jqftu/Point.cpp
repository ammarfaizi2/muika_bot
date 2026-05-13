// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Point.hpp>
#include <muika/helpers.hpp>

#include <stdexcept>
#include <string>
#include <cstdio>

namespace muika {
namespace modules {
namespace m002_jqftu {

using json = ::nlohmann::json;

// static
std::string Point::chatPointsDir(const std::string &points_dir, int64_t chat_id)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "/s_%lld", (long long)chat_id);
	return points_dir + buf;
}

json Point::toJson(void) const
{
	return {
		{"user_id", user_id_},
		{"point", point_},
		{"username", username_},
		{"first_name", first_name_},
		{"last_name", last_name_}
	};
}

std::string Point::toJsonString(void) const
{
	return toJson().dump(1, '\t');
}

void Point::fromJson(const json &j)
{
	if (!j.is_object())
		throw std::runtime_error("JSON is not an object");

	if (!j.contains("user_id") || !j["user_id"].is_number_unsigned())
		throw std::runtime_error("JSON does not contain a valid user_id");

	if (!j.contains("point") || !j["point"].is_number_unsigned())
		throw std::runtime_error("JSON does not contain a valid point");

	if (!j.contains("username") || !j["username"].is_string())
		throw std::runtime_error("JSON does not contain a valid username");

	if (!j.contains("first_name") || !j["first_name"].is_string())
		throw std::runtime_error("JSON does not contain a valid first_name");

	if (!j.contains("last_name") || !j["last_name"].is_string())
		throw std::runtime_error("JSON does not contain a valid last_name");

	setUserId(j["user_id"].get<uint64_t>());
	setPoint(j["point"].get<uint64_t>());
	setUsername(j["username"].get<std::string>());
	setFirstName(j["first_name"].get<std::string>());
	setLastName(j["last_name"].get<std::string>());
}

void Point::fromJsonString(const std::string &s)
{
	try {
		fromJson(json::parse(s));
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to parse JSON string: " +
					 std::string(e.what()));
	}
}

void Point::saveToDisk(const std::string &points_dir, int64_t chat_id) const
{
	std::string dir = chatPointsDir(points_dir, chat_id);
	if (mk_mkdir_p(dir.c_str(), 0755) < 0)
		throw std::runtime_error("Failed to create points dir: " + dir);

	char tail[64];
	snprintf(tail, sizeof(tail), "/%llu.json",
		 (unsigned long long)user_id_);
	std::string path = dir + tail;

	int r = mk_file_put_contents(path, toJsonString());
	if (r < 0)
		throw std::runtime_error("Failed to write points file: " + path);
}

// static
Point Point::tryLoadFromDisk(const std::string &points_dir,
			     int64_t chat_id, uint64_t user_id)
{
	char tail[64];
	snprintf(tail, sizeof(tail), "/%llu.json", (unsigned long long)user_id);
	std::string path = chatPointsDir(points_dir, chat_id) + tail;

	std::string contents;
	if (mk_file_get_contents(path, contents) < 0)
		return Point();

	Point p;
	p.fromJsonString(contents);
	return p;
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
