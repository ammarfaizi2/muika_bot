// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP
#define MUIKA__MODULES__JQFTU__SESSIONMAP_HPP

#include <muika/Muika.hpp>
#include <muika/modules/jqftu/Session.hpp>

#include <unordered_map>
#include <mutex>

namespace muika {
namespace modules {
namespace jqftu {

class SessionMap {
private:
	Muika &m_;
	std::mutex mutex_;
	std::unordered_map<int64_t, Session> smap_;

public:
	SessionMap(Muika &m);
	~SessionMap(void);

	void loadAll(void);
	void load(int64_t chat_id);
	void save(void);
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP */
