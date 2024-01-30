// SPDX-License-Identifier: GPL-2.0-only

#include "muika/modules/jqftu/Point.hpp"

#include <stdexcept>
#include <string>
#include <cstdio>

using json = ::nlohmann::json;

namespace muika {
namespace modules {
namespace jqftu {

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
		throw std::runtime_error("Failed to parse JSON string: " + std::string(e.what()));
	}
}

void Point::saveToDisk(int64_t chat_id) const
{
	char path[4096];
	size_t len;
	FILE *fp;

	snprintf(path, sizeof(path), "./storage/jqftu/points/s_%lld/%llu.json",
		 (long long)chat_id, (unsigned long long)user_id_);

	fp = fopen(path, "wb");
	if (!fp)
		throw std::runtime_error("Failed to open file for writing");

	try {
		std::string json_str = toJsonString();
		len = fwrite(json_str.c_str(), 1, json_str.size(), fp);
		fclose(fp);
		if (len != json_str.size())
			throw std::runtime_error("fwrite() error");
	} catch (const std::exception &e) {
		fclose(fp);
		throw e;
	}
}

// static
Point Point::tryLoadFromDisk(int64_t chat_id, uint64_t user_id)
{
	char path[4096];
	size_t len;
	FILE *fp;

	snprintf(path, sizeof(path), "./storage/jqftu/points/s_%lld/%llu.json",
		 (long long)chat_id, (unsigned long long)user_id);

	fp = fopen(path, "rb");
	if (!fp)
		return Point();

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	try {
		size_t tmp;

		std::string json_str(len, '\0');
		tmp = fread(&json_str[0], 1, len, fp);
		fclose(fp);
		if (tmp != len)
			throw std::runtime_error("fread() error");

		Point p;
		p.fromJsonString(json_str);
		return p;
	} catch (const std::exception &e) {
		fclose(fp);
		throw e;
	}
}

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */
