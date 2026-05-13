// SPDX-License-Identifier: GPL-2.0-only
#include <muika/tests/test.hpp>

#include <string>

int main(int argc, char **argv)
{
	std::string filter;
	if (argc > 1)
		filter = argv[1];

	return ::muika::tests::Registry::instance().run(filter);
}
