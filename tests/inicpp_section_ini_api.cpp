#include "../inicpp.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>

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

	bool contains_section(const std::list<std::string> &sections, const std::string &name)
	{
		for (std::list<std::string>::const_iterator it = sections.begin(); it != sections.end(); ++it)
		{
			if (*it == name)
			{
				return true;
			}
		}
		return false;
	}

	void test_section_direct_api()
	{
		inicpp::section section;
		CHECK_TRUE(section.isEmpty());
		CHECK_EQ(std::string(""), section.name());

		section.setName("server", 10);
		section.setValue("port", "8080", 11);
		section.setValue("pi", "3.5", 12);

		CHECK_EQ(std::string("server"), section.name());
		CHECK_TRUE(!section.isEmpty());
		CHECK_TRUE(section.isKeyExists("port"));
		CHECK_TRUE(!section.isKeyExists("missing"));
		CHECK_TRUE(section.isKeyExist("port"));
		CHECK_TRUE(!section.isKeyExist("missing"));
		CHECK_EQ(std::string("8080"), section.getValue("port"));
		CHECK_EQ(std::string(""), section.getValue("missing"));
		CHECK_EQ(8080, section.toInt("port"));
		CHECK_TRUE(std::fabs(section.toDouble("pi") - 3.5) < 0.0001);
		CHECK_EQ(std::string("8080"), section.toString("port"));
		CHECK_EQ(11, section.getLine("port"));
		CHECK_EQ(-1, section.getLine("missing"));
		CHECK_EQ(12, section.getEndSection());

		section["mode"] = std::string("prod");
		CHECK_EQ(std::string("prod"), section.toString("mode"));

		std::map<std::string, std::string> values = section.getSectionMap();
		CHECK_EQ(std::string("8080"), values["port"]);
		CHECK_EQ(std::string("prod"), values["mode"]);

		section.clear();
		CHECK_TRUE(section.isEmpty());
		CHECK_EQ(std::string(""), section.name());
		CHECK_EQ(-1, section.getEndSection());
	}

	void test_ini_direct_api()
	{
		inicpp::section server("server");
		server.setValue("port", "8080", 3);

		inicpp::ini data;
		CHECK_TRUE(data.empty());
		data.addSection(server);

		CHECK_TRUE(!data.empty());
		CHECK_TRUE(data.isSectionExists("server"));
		CHECK_TRUE(!data.isSectionExists("missing"));
		CHECK_EQ(static_cast<std::size_t>(1), data.getSectionSize());
		CHECK_EQ(std::string("8080"), data.getValue("server", "port"));
		CHECK_EQ(std::string(""), data.getValue("missing", "port"));
		CHECK_EQ(3, data.getLine("server", "port"));
		CHECK_EQ(-1, data.getLine("server", "missing"));
		CHECK_EQ(std::string("8080"), data["server"].toString("port"));

		std::map<std::string, std::string> values = data.getSectionMap("server");
		CHECK_EQ(std::string("8080"), values["port"]);
		CHECK_TRUE(data.getSectionMap("missing").empty());
		CHECK_TRUE(contains_section(data.getSectionsList(), "server"));
	}

	void test_ini_unnamed_section_lines_and_listing()
	{
		inicpp::section unnamed("");
		unnamed.setValue("title", "config.ini", 1);

		inicpp::ini data;
		data.addSection(unnamed);

		CHECK_TRUE(data.isSectionExists(""));
		CHECK_EQ(std::string("config.ini"), data.getValue("", "title"));
		CHECK_EQ(1, data.getLine("title"));
		CHECK_TRUE(contains_section(data.getSectionsList(), ""));
	}

	void test_ini_duplicate_section_merge_uses_last_value()
	{
		inicpp::section first("app");
		first.setValue("shared", "first", 1);
		first.setValue("first_only", "yes", 2);

		inicpp::section second("app");
		second.setValue("shared", "second", 3);
		second.setValue("second_only", "yes", 4);

		inicpp::ini data;
		data.addSection(first);
		data.addSection(second);

		CHECK_EQ(std::string("second"), data.getValue("app", "shared"));
		CHECK_EQ(std::string("yes"), data.getValue("app", "first_only"));
		CHECK_EQ(std::string("yes"), data.getValue("app", "second_only"));
	}

	void test_ini_remove_clear_and_empty()
	{
		inicpp::section server("server");
		server.setValue("port", "8080", 1);

		inicpp::ini data;
		data.addSection(server);
		data.removeSection("missing");
		CHECK_TRUE(data.isSectionExists("server"));

		data.removeSection("server");
		CHECK_TRUE(!data.isSectionExists("server"));
		CHECK_TRUE(data.empty());

		data.addSection(server);
		CHECK_TRUE(!data.empty());
		data.clear();
		CHECK_TRUE(data.empty());
	}
}

int main()
{
	test_section_direct_api();
	test_ini_direct_api();
	test_ini_unnamed_section_lines_and_listing();
	test_ini_duplicate_section_merge_uses_last_value();
	test_ini_remove_clear_and_empty();
	std::cout << "inicpp_section_ini_api: PASS" << std::endl;
	return 0;
}
