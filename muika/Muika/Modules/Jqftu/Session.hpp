// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA__MODULES__JQFTU__SESSION_HPP
#define MUIKA__MUIKA__MODULES__JQFTU__SESSION_HPP

#include <muika/Muika/Modules/Jqftu/ModJqftu.hpp>
#include <memory>
#include <mutex>

namespace Muika {
namespace Modules {
namespace Jqftu {

class ModJqftu;

class Session {
public:
	Session(ModJqftu *mj);
	~Session(void);

	static std::unique_ptr<Session> loadFromFile(ModJqftu *mj, const std::string &path);
	bool saveToFile(const std::string &path);
	inline std::mutex &getMtx(void) { return mtx_; }

	inline void initData(int64_t chat_id, const std::string &chat_title,
			     uint64_t sess_last_msg_id)
	{
		chat_id_ = chat_id;
		chat_title_ = chat_title;
		sess_last_msg_id_ = sess_last_msg_id;
	}

private:
	ModJqftu *mj_;
	std::mutex mtx_;

	__json int64_t chat_id_;
	__json std::string chat_title_;
	__json uint64_t sess_last_msg_id_;
};

} /* namespace Jqftu */
} /* namespace Modules */
} /* namespace Muika */

#endif
