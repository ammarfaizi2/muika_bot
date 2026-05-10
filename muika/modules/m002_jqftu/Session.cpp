// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Session.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {

Module::Type Session::passMsg(MsgPtr msg)
{
	return Module::Type::MSG_SKIP;
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
