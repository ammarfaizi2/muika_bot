// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP
#define MUIKA__MODULES__JQFTU__ENTRY_HPP

#include <muika/helpers.hpp>

#include <mutex>

namespace muika {
namespace modules {
namespace jqftu {

class User {
private:
	__serialize int64_t chat_id_;
	__serialize uint64_t id_;
	__serialize std::string first_name_;
	__serialize std::string last_name_;
	__serialize std::string username_;
	__serialize int64_t point_;

public:
	inline User(int64_t chat_id, uint64_t id, std::string first_name,
		    std::string last_name, std::string username, int64_t point):
		chat_id_(chat_id),
		id_(id),
		first_name_(first_name),
		last_name_(last_name),
		username_(username),
		point_(point)
	{
	}

	inline int64_t chat_id(void) { return chat_id_; }
	inline uint64_t id(void) { return id_; }
	inline std::string first_name(void) { return first_name_; }
	inline std::string last_name(void) { return last_name_; }
	inline std::string username(void) { return username_; }
	inline int64_t point(void) { return point_; }

	inline void addPoint(int64_t point) { point_ += point; }
	inline void subPoint(int64_t point) { point_ -= point; }
	inline void setPoint(int64_t point) { point_ = point; }
	inline void setFirstName(std::string first_name) { first_name_ = first_name; }
	inline void setLastName(std::string last_name) { last_name_ = last_name; }
	inline void setUsername(std::string username) { username_ = username; }
	inline void setId(uint64_t id) { id_ = id; }

	using json = nlohmann::json;

	json toJson(void) const;
	std::string serialize(void) const;

	static json toJson(const User &user);
	static User fromJson(const json &j);
	static std::string serialize(const User &user);
	static User deserialize(const std::string &str);
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP */
