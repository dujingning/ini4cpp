#include "../inicpp.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace
{
	void fail(const std::string &message, const char *file, int line)
	{
		std::cerr << file << ":" << line << ": " << message << std::endl;
		std::exit(1);
	}

#define CHECK_TRUE(expr)                                                           \
	do                                                                             \
	{                                                                              \
		if (!(expr))                                                               \
		{                                                                          \
			fail(std::string("CHECK_TRUE failed: ") + #expr, __FILE__, __LINE__); \
		}                                                                          \
	} while (false)

#define CHECK_EQ(expected, actual)                                                 \
	do                                                                             \
	{                                                                              \
		if (!((expected) == (actual)))                                              \
		{                                                                          \
			std::ostringstream oss;                                                \
			oss << "CHECK_EQ failed: expected [" << (expected) << "], actual ["    \
				<< (actual) << "]";                                               \
			fail(oss.str(), __FILE__, __LINE__);                                  \
		}                                                                          \
	} while (false)

	void test_integer_conversions()
	{
		std::string value = "42";
		inicpp::ValueProxy proxy(value);

		CHECK_EQ(static_cast<short>(42), static_cast<short>(proxy));
		CHECK_EQ(42, static_cast<int>(proxy));
		CHECK_EQ(42L, static_cast<long>(proxy));
		CHECK_EQ(42LL, static_cast<long long>(proxy));
		CHECK_EQ(static_cast<unsigned short>(42), static_cast<unsigned short>(proxy));
		CHECK_EQ(42U, static_cast<unsigned int>(proxy));
		CHECK_EQ(42UL, static_cast<unsigned long>(proxy));
		CHECK_EQ(42ULL, static_cast<unsigned long long>(proxy));
	}

	void test_character_conversions()
	{
		std::string charValue = "A";
		inicpp::ValueProxy charProxy(charValue);
		CHECK_EQ('A', static_cast<char>(charProxy));

		std::string unsignedCharValue = "Q";
		inicpp::ValueProxy unsignedCharProxy(unsignedCharValue);
		CHECK_TRUE(static_cast<unsigned char>(unsignedCharProxy) == static_cast<unsigned char>('Q'));
	}

	void test_floating_conversions()
	{
		std::string value = "3.5";
		inicpp::ValueProxy proxy(value);

		CHECK_TRUE(std::fabs(static_cast<float>(proxy) - 3.5F) < 0.0001F);
		CHECK_TRUE(std::fabs(static_cast<double>(proxy) - 3.5) < 0.0001);
	}

	void test_bool_conversion_rules()
	{
		std::string zero = "0";
		std::string falseValue = "false";
		std::string no = "no";
		std::string one = "1";
		std::string trueValue = "true";
		std::string upperFalse = "False";

		CHECK_TRUE(!static_cast<bool>(inicpp::ValueProxy(zero)));
		CHECK_TRUE(!static_cast<bool>(inicpp::ValueProxy(falseValue)));
		CHECK_TRUE(!static_cast<bool>(inicpp::ValueProxy(no)));
		CHECK_TRUE(static_cast<bool>(inicpp::ValueProxy(one)));
		CHECK_TRUE(static_cast<bool>(inicpp::ValueProxy(trueValue)));
		CHECK_TRUE(static_cast<bool>(inicpp::ValueProxy(upperFalse)));
	}

	void test_string_access_and_stream_output()
	{
		std::string value = "hello world";
		inicpp::ValueProxy proxy(value);

		CHECK_EQ(std::string("hello world"), static_cast<std::string>(proxy));
		CHECK_EQ(std::string("hello world"), proxy.get<std::string>());
		CHECK_EQ(std::string("hello world"), proxy.String());

		std::ostringstream output;
		output << proxy;
		CHECK_EQ(std::string("hello world"), output.str());
	}

	void test_assignment_updates_referenced_value()
	{
		std::string value = "old";
		inicpp::ValueProxy proxy(value);

		proxy = 123;
		CHECK_EQ(std::string("123"), value);

		proxy = std::string("new value");
		CHECK_EQ(std::string("new value"), value);

		proxy = 'Z';
		CHECK_EQ(std::string("Z"), value);
	}

	void test_to_string_helper()
	{
		CHECK_EQ(std::string("123"), inicpp::ValueProxy::to_string(123));
		CHECK_EQ(std::string("A"), inicpp::ValueProxy::to_string('A'));
	}

	void test_value_proxy_construction_contract()
	{
		static_assert(std::is_constructible<inicpp::ValueProxy, std::string &>::value,
					  "ValueProxy should wrap a writable std::string reference");
		static_assert(!std::is_constructible<inicpp::ValueProxy, int>::value,
					  "ValueProxy should not expose the broken value-copy constructor");
	}

	void test_invalid_conversion_throws()
	{
		std::string value = "not-a-number";
		inicpp::ValueProxy proxy(value);

		bool threw = false;
		try
		{
			(void)proxy.get<int>();
		}
		catch (const std::runtime_error &)
		{
			threw = true;
		}

		CHECK_TRUE(threw);
	}
}

int main()
{
	test_integer_conversions();
	test_character_conversions();
	test_floating_conversions();
	test_bool_conversion_rules();
	test_string_access_and_stream_output();
	test_assignment_updates_referenced_value();
	test_to_string_helper();
	test_value_proxy_construction_contract();
	test_invalid_conversion_throws();
	std::cout << "inicpp_value_proxy: PASS" << std::endl;
	return 0;
}
