// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__SESSION_HPP
#define MUIKA__MODULES__M002_JQFTU__SESSION_HPP

#include <muika/Module.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {

class Session {
public:
	Module::Type passMsg(MsgPtr msg);

private:
	std::string chat_id_;
};

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__M002_JQFTU__SESSION_HPP */
