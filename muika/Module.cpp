// SPDX-License-Identifier: GPL-2.0-only
#include <muika/Module.hpp>

namespace muika {

Module::Module(const std::string &name, const std::string &desc):
	name_(name),
	desc_(desc)
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

void Module::setMk(Muika *mk)
{
	mk_ = mk;
}

Reactor *Module::reactor(void)
{
	return mk_->reactor();
}

void Module::setStorageDir(const std::string &dir)
{
	storage_dir_ = dir;
}

const std::string &Module::storageDir(void) const
{
	return storage_dir_;
}

} /* namespace muika */
