// SPDX-License-Identifier: GPL-2.0-only
#include <muika/Module.hpp>

namespace muika {

Module::Module(const std::string &name):
	name_(name)
{
}

const std::string& Module::moduleName(void) const
{
	return name_;
}

void Module::init(void)
{
}

void Module::free(void)
{
}

void Module::setEnabled(bool on)
{
	enabled_ = on;
}

bool Module::enabled(void) const
{
	return enabled_;
}

Muika *Module::mk(void) const
{
	return mk_;
}

} /* namespace muika */
