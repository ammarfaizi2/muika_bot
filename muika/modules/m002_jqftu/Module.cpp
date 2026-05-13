// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Module.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {

class Internal {
public:
	inline Internal(Module *m): m_(m) {}
	inline Module::Type passMsg(MsgPtr) { return Module::Type::MSG_SKIP; }

private:
	Module *m_;
};

Module::Module(void):
	muika::Module("jqftu", "Japanese Quiz")
{
	internal_ = std::make_unique<Internal>(this);
}

Module::Type Module::passMsg(MsgPtr msg)
{
	return internal_->passMsg(msg);
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
