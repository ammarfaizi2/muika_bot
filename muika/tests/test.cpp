// SPDX-License-Identifier: GPL-2.0-only
#include <muika/tests/test.hpp>

#include <cstdio>
#include <exception>
#include <utility>

namespace muika {
namespace tests {

Registry &Registry::instance(void)
{
	static Registry r;
	return r;
}

void Registry::add(const TestCase &tc)
{
	cases_.push_back(tc);
}

int Registry::run(const std::string &filter)
{
	int passed = 0;
	int failed = 0;
	int skipped = 0;

	for (const auto &tc : cases_) {
		std::string full = tc.suite + "." + tc.name;

		if (!filter.empty() && full.find(filter) == std::string::npos) {
			++skipped;
			continue;
		}

		printf("[ RUN      ] %s\n", full.c_str());
		try {
			tc.fn();
			printf("[       OK ] %s\n", full.c_str());
			++passed;
		} catch (const AssertionError &e) {
			printf("[  FAILED  ] %s\n    %s\n", full.c_str(), e.what());
			++failed;
		} catch (const std::exception &e) {
			printf("[  FAILED  ] %s\n    unexpected exception: %s\n",
			       full.c_str(), e.what());
			++failed;
		} catch (...) {
			printf("[  FAILED  ] %s\n    unknown exception\n",
			       full.c_str());
			++failed;
		}
	}

	printf("\n");
	printf("Test results: %d passed, %d failed, %d skipped (total %zu)\n",
	       passed, failed, skipped, cases_.size());
	return failed == 0 ? 0 : 1;
}

Registrar::Registrar(const std::string &suite, const std::string &name,
		     std::function<void(void)> fn)
{
	Registry::instance().add({suite, name, std::move(fn)});
}

} /* namespace tests */
} /* namespace muika */
