// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP
#define MUIKA__MODULES__JQFTU__SESSIONMAP_HPP

#include <muika/Muika.hpp>

namespace muika {
namespace modules {
namespace jqftu {

class SessionMap {
private:
	Muika &m_;

public:
	inline SessionMap(Muika &m):
		m_(m)
	{
	}

	~SessionMap(void) = default;

	void load(void);
	void save(void);
};

} /* namespace jqftu */
} /* namespace modules */
} /* namespace muika */

#endif /* #ifndef MUIKA__MODULES__JQFTU__SESSIONMAP_HPP */
