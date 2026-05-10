// SPDX-License-Identifier: GPL-2.0-only
#include <muika/modules/m002_jqftu/Module.hpp>
#include <muika/modules/m002_jqftu/Session.hpp>
#include <unordered_map>

namespace muika {
namespace modules {
namespace m002_jqftu {

class Internal {
public:
	inline Internal(Module *m);
	inline Module::Type passMsg(MsgPtr msg);

private:
	Module *m_;
	std::unordered_map<std::string, std::unique_ptr<Session>> sessions_;
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

inline Internal::Internal(Module *m):
	m_(m)
{
}

inline Module::Type Internal::passMsg(MsgPtr msg)
{
	auto chat_id = msg->chat_id();
	auto it = sessions_.find(chat_id);
	if (it == sessions_.end())
		return Module::Type::MSG_SKIP;

	return it->second->passMsg(msg);
}

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */
