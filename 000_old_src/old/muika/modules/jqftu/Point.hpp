// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__POINT_HPP
#define MUIKA__MODULES__JQFTU__POINT_HPP

#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>
#include <nlohmann/json.hpp>

namespace muika {
namespace modules {
namespace jqftu {

class Point {
private:
	uint64_t user_id_ = 0;
	uint64_t point_ = 0;
	std::string username_;
	std::string first_name_;
	std::string last_name_;

public:
	using json = ::nlohmann::json;
	Point(void) = default;
	~Point(void) = default;

	Point(uint64_t user_id, uint64_t points, const std::string &username,
	      const std::string &first_name, const std::string &last_name):
		user_id_(user_id),
		point_(points),
		username_(username),
		first_name_(first_name),
		last_name_(last_name)
	{
	}

	inline uint64_t getUserId(void) const { return user_id_; }
	inline uint64_t getPoint(void) const { return point_; }
	inline const std::string &getUsername(void) const { return username_; }
	inline const std::string &getFirstName(void) const { return first_name_; }
	inline const std::string &getLastName(void) const { return last_name_; }
	inline std::string getFullName(void) const
	{
		return first_name_ + (last_name_.empty() ? "" : " " + last_name_);
	}

	inline void setUserId(uint64_t user_id) { user_id_ = user_id; }
	inline void setPoint(uint64_t point) { point_ = point; }
	inline void setUsername(const std::string &username) { username_ = username; }
	inline void setFirstName(const std::string &first_name) { first_name_ = first_name; }
	inline void setLastName(const std::string &last_name) { last_name_ = last_name; }

	inline void addPoint(uint64_t point) { point_ += point; }
	inline void subPoint(uint64_t point) { point_ -= point; }
	inline void mulPoint(uint64_t point) { point_ *= point; }

	json toJson(void) const;
	std::string toJsonString(void) const;
	void saveToDisk(int64_t chat_id) const;

	void fromJson(const json &j);
	void fromJsonString(const std::string &s);
	static Point tryLoadFromDisk(int64_t chat_id, uint64_t user_id);
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__POINT_HPP */
