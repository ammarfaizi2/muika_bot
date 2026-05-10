// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M002_JQFTU__MODULE_HPP
#define MUIKA__MODULES__M002_JQFTU__MODULE_HPP

#include <muika/Module.hpp>

namespace muika {
namespace modules {
namespace m002_jqftu {

class Internal;

class Module: public muika::Module {
public:
	Module();
	virtual ~Module() = default;
	virtual Type passMsg(MsgPtr msg) override;

private:
	std::unique_ptr<Internal> internal_;
	friend class Internal;
};

} /* namespace m002_jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__M002_JQFTU__MODULE_HPP */
