// SPDX-License-Identifier: GPL-2.0-only
#ifndef MUIKA__TESTS__TEST_HPP
#define MUIKA__TESTS__TEST_HPP

#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace muika {
namespace tests {

struct TestCase {
	std::string suite;
	std::string name;
	std::function<void(void)> fn;
};

class Registry {
public:
	static Registry &instance(void);
	void add(const TestCase &tc);
	int run(const std::string &filter = "");

private:
	std::vector<TestCase> cases_;
};

class AssertionError: public std::exception {
public:
	explicit AssertionError(std::string m): msg_(std::move(m)) {}
	const char *what(void) const noexcept override { return msg_.c_str(); }

private:
	std::string msg_;
};

struct Registrar {
	Registrar(const std::string &suite, const std::string &name,
		  std::function<void(void)> fn);
};

} /* namespace tests */
} /* namespace muika */

#define MK_TEST(suite, name)								\
	static void mk_test_##suite##_##name(void);					\
	static ::muika::tests::Registrar						\
		mk_test_reg_##suite##_##name(#suite, #name,				\
					     mk_test_##suite##_##name);			\
	static void mk_test_##suite##_##name(void)

#define MK_FAIL(msg) do {								\
	std::ostringstream __mk_oss;							\
	__mk_oss << __FILE__ << ":" << __LINE__ << ": " << msg;				\
	throw ::muika::tests::AssertionError(__mk_oss.str());				\
} while (0)

#define MK_ASSERT(cond) do {								\
	if (!(cond))									\
		MK_FAIL("MK_ASSERT(" #cond ") failed");					\
} while (0)

#define MK_ASSERT_TRUE(cond)  MK_ASSERT(cond)
#define MK_ASSERT_FALSE(cond) MK_ASSERT(!(cond))

#define MK_ASSERT_EQ(a, b) do {								\
	auto __mk_a = (a);								\
	auto __mk_b = (b);								\
	if (!(__mk_a == __mk_b)) {							\
		std::ostringstream __mk_oss;						\
		__mk_oss << "MK_ASSERT_EQ(" #a ", " #b ") failed: "			\
			 << __mk_a << " != " << __mk_b;					\
		MK_FAIL(__mk_oss.str());						\
	}										\
} while (0)

#define MK_ASSERT_NE(a, b) do {								\
	auto __mk_a = (a);								\
	auto __mk_b = (b);								\
	if (!(__mk_a != __mk_b)) {							\
		std::ostringstream __mk_oss;						\
		__mk_oss << "MK_ASSERT_NE(" #a ", " #b ") failed: both = "		\
			 << __mk_a;							\
		MK_FAIL(__mk_oss.str());						\
	}										\
} while (0)

#endif /* MUIKA__TESTS__TEST_HPP */
