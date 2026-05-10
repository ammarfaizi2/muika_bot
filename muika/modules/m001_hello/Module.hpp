// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__M001_HELLO__MODULE_HPP
#define MUIKA__MODULES__M001_HELLO__MODULE_HPP

#include <muika/Module.hpp>

namespace muika {
namespace modules {
namespace m001_hello {

class Module: public muika::Module {
public:
	Module();
	virtual ~Module() = default;
	virtual Type passMsg(MsgPtr msg) override;
};

} /* namespace m001_hello */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__M001_HELLO__MODULE_HPP */
