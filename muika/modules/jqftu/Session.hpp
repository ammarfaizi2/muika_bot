// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__SESSION_HPP
#define MUIKA__MODULES__JQFTU__SESSION_HPP

#include <muika/Muika.hpp>

#include <mutex>

namespace muika {
namespace modules {
namespace jqftu {

class Session {
private:
	Muika &m_;

	std::mutex mutex_;
	volatile bool should_stop_ = false;
	bool silent_start_ = false;

	__serialize int64_t chat_id_;
	__serialize uint64_t last_msg_id_ = 0;
	__serialize uint32_t timeout_secs_ = 3600;
	__serialize uint32_t next_card_delay_secs_ = 5;

public:
	Session(Muika& muika, int64_t chat_id, const std::vector<std::string> &deck_list);
	~Session(void);
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__SESSION_HPP */
