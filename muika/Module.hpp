// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULE_HPP
#define MUIKA__MODULE_HPP

#include <muika/Message.hpp>
#include <memory>

namespace muika {

typedef std::shared_ptr<Message> MsgPtr;

class Muika;

class Module {
public:
	enum class Type {
		MSG_SKIP,
		MSG_HANDLED,
	};

	Module(const std::string &name, const std::string &desc = "");
	virtual ~Module() = default;
	virtual void init(void);
	virtual void free(void);
	virtual Type passMsg(MsgPtr msg) = 0;
	void setEnabled(bool on);
	bool enabled(void) const;
	const std::string& moduleName(void) const;
	Muika *mk(void) const;
	void setMk(Muika *mk);

private:
	Muika *mk_ = nullptr;
	std::string name_;
	std::string desc_ = "";
	bool enabled_ = true;
	Module(const Module&) = delete;
	Module& operator=(const Module&) = delete;
};

} /* namespace muika */

#include <muika/Muika.hpp>

#endif /* #ifndef MUIKA__MODULE_HPP */
