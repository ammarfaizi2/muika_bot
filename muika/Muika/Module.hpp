// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MUIKA__MODULE_HPP
#define MUIKA__MUIKA__MODULE_HPP

#include <muika/Muika/MuikaBot.hpp>
#include <tgbot/tgbot.h>

namespace Muika {

enum {
	MOD_INVOKE_OK = 0,
	MOD_INVOKE_SKIP = 1,
	MOD_INVOKE_ERR = 2,
};

class Module {
public:
	Module(MuikaBot *mb): mb_(mb) {}
	virtual ~Module(void) = default;
	virtual int init(void) = 0;
	virtual void shutdown(void) = 0;
	virtual int invoke(TgBot::Message::Ptr &msg) = 0;
	void setName(const char *name);
	inline const char *getName(void) const { return mod_name_; }

protected:
	MuikaBot *mb_;

private:
	char mod_name_[32];
};

} /* namespace Muika */

#endif
