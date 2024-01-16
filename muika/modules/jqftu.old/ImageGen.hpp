// SPDX-License-Identifier: GPL-2.0-only

#ifndef MUIKA__IMAGEGEN_HPP
#define MUIKA__IMAGEGEN_HPP

#include <string>

namespace muika {
namespace modules {
namespace jqftu {

class ImageGen {
private:
	std::string text_;
	std::string hash_;
	int generateImage(void);

public:
	ImageGen(const std::string &text);
	~ImageGen(void);
	int generate(void);
	const std::string getURL(void) const;
};

} /* namespace muika::modules::jqftu */
} /* namespace muika::modules */
} /* namespace muika */

#endif /* MUIKA__IMAGEGEN_HPP */
