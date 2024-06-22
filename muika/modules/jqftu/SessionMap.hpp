// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP
#define MUIKA__MODULES__JQFTU__SESSIONMAP_HPP

#include <muika/Muika.hpp>
#include <muika/modules/jqftu/Session.hpp>

#include <unordered_map>
#include <memory>
#include <mutex>

namespace muika {
namespace modules {
namespace jqftu {

class SessionMap {
private:
	Muika &m_;
	std::mutex mutex_;
	std::unordered_map<int64_t, std::shared_ptr<Session>> sessions_;

public:
	SessionMap(Muika &m);
	~SessionMap(void);

	void loadAll(void);
	void load(int64_t chat_id);
	void save(void);
	std::shared_ptr<Session> create(int64_t chat_id, const std::vector<std::string> &deck_list);
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP */
