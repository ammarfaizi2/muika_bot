// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP
#define MUIKA__MODULES__JQFTU__ENTRY_HPP

#include <tgbot/tgbot.h>

#include <string>
#include <memory>

namespace muika {
namespace modules {
namespace jqftu {

class entry {
public:
	Foundation(void);
	virtual ~Foundation(void);
	virtual void handleMessage(TgBot::Message::Ptr &msg) = 0;
	virtual void init(void) = 0;
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__ENTRY_HPP */
