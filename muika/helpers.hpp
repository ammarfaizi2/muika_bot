// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__HELPERS_HPP
#define MUIKA__HELPERS_HPP

#include <string>
#include <cstddef>

#ifdef __CHECKER__
#ifndef __must_hold
#define __must_hold(x)	__attribute__((__context__(x,1,1)))
#endif

#ifndef __acquires
#define __acquires(x)	__attribute__((__context__(x,0,1)))
#endif

#ifndef __releases
#define __releases(x)	__attribute__((__context__(x,1,0)))
#endif

#ifndef __serialize
#define __serialize	__attribute__((__serialize__))
#endif
#else /* __CHECKER__ */
#ifndef __must_hold
#define __must_hold(x)
#endif

#ifndef __acquires
#define __acquires(x)
#endif

#ifndef __releases
#define __releases(x)
#endif

#ifndef __serialize
#define __serialize
#endif
#endif /* __CHECKER__ */

namespace muika {

void strtolower(std::string &str);
void strtoupper(std::string &str);
void str_replace(std::string &str, const std::string &from,
		 const std::string &to, size_t *count = nullptr);
std::string normalize_answer(const std::string &str);
std::string htmlspecialchars(const std::string &str);
size_t similar_text(const std::string &a, const std::string &b);

} /* namespace muika */

#endif /* MUIKA__HELPERS_HPP */
