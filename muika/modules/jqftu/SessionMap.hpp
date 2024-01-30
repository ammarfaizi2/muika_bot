// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP
#define MUIKA__MODULES__JQFTU__SESSIONMAP_HPP

#include "muika/Muika.hpp"
#include "muika/helpers.hpp"
#include "muika/modules/jqftu/Session.hpp"

#include <unordered_map>
#include <memory>
#include <mutex>

namespace muika {
namespace modules {
namespace jqftu {

class SessionMap {
private:
	std::mutex mutex_;
	std::unordered_map<int64_t, std::shared_ptr<Session>> sessions_;

public:
	SessionMap(void) = default;
	~SessionMap(void) = default;

	__must_hold(&mutex_) std::shared_ptr<Session> __find(int64_t chat_id);
	__must_hold(&mutex_) bool __remove(int64_t chat_id);
	__must_hold(&mutex_) void __insert(int64_t chat_id, std::shared_ptr<Session> session);

	std::shared_ptr<Session> find(int64_t chat_id);
	bool remove(int64_t chat_id);
	void insert(int64_t chat_id, std::shared_ptr<Session> session);
	void loadSessionsFromDisk(Muika &m);

	inline std::mutex &getMutex(void) { return mutex_; }
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__MODULES__JQFTU__SESSIONMAP_HPP */
