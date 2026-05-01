#include "../inicpp.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>

namespace
{
	void fail(const std::string &message, const char *file, int line)
	{
		std::cerr << file << ":" << line << ": " << message << std::endl;
		std::exit(1);
	}

#define CHECK_TRUE(expr)                                                              \
	do                                                                                \
	{                                                                                 \
		if (!(expr))                                                                  \
		{                                                                             \
			fail(std::string("CHECK_TRUE failed: ") + #expr, __FILE__, __LINE__);    \
		}                                                                             \
	} while (false)

#define CHECK_EQ(expected, actual)                                                    \
	do                                                                                \
	{                                                                                 \
		if (!((expected) == (actual)))                                                 \
		{                                                                             \
			std::ostringstream oss;                                                   \
			oss << "CHECK_EQ failed: expected [" << (expected) << "], actual ["      \
				<< (actual) << "]";                                                  \
			fail(oss.str(), __FILE__, __LINE__);                                      \
		}                                                                             \
	} while (false)

	std::string temp_path(const std::string &name)
	{
		static std::random_device random;
		static unsigned long counter = 0;

		for (int attempt = 0; attempt < 16; ++attempt)
		{
			std::ostringstream oss;
			oss << "inicpp_ini_format_" << name << "_" << random() << "_" << random()
				<< "_" << counter++ << ".ini";
			const std::string path = oss.str();

			std::ifstream existing(path.c_str());
			if (!existing.good())
			{
				return path;
			}
		}

		fail(std::string("failed to create unique temp path for: ") + name, __FILE__, __LINE__);
		return "";
	}

	void write_file(const std::string &path, const std::string &content)
	{
		std::ofstream output(path.c_str(), std::ofstream::out | std::ofstream::trunc);
		CHECK_TRUE(output.is_open());
		output << content;
		output.close();
		CHECK_TRUE(static_cast<bool>(output));
	}

	void test_leading_whitespace_comments_are_ignored()
	{
		const std::string path = temp_path("leading_comments");
		write_file(path, "   ; comment\n\t# comment\n[main]\nkey=value\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("value"), ini["main"].toString("key"));
		CHECK_TRUE(!ini[""].isKeyExist("# comment"));

		std::remove(path.c_str());
	}

	void test_section_header_allows_leading_and_inner_spaces()
	{
		const std::string path = temp_path("section_spaces");
		write_file(path, "   [ server name ]   \nport = 8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.isSectionExists("server name"));
		CHECK_EQ(std::string("8080"), ini["server name"].toString("port"));

		std::remove(path.c_str());
	}

	void test_section_header_allows_inline_comment()
	{
		const std::string path = temp_path("section_inline_comment");
		write_file(path, "[server] ; production server\nhost=localhost\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("localhost"), ini["server"].toString("host"));

		std::remove(path.c_str());
	}

	void test_inline_comment_is_stripped_when_preceded_by_space()
	{
		const std::string path = temp_path("inline_comments");
		write_file(path, "[main]\nname=value ; comment\npath=/tmp/a # comment\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("value"), ini["main"].toString("name"));
		CHECK_EQ(std::string("/tmp/a"), ini["main"].toString("path"));

		std::remove(path.c_str());
	}

	void test_inline_comment_markers_inside_values_are_preserved()
	{
		const std::string path = temp_path("comment_markers_in_values");
		write_file(path,
				   "[main]\n"
				   "url=http://example.test/a#fragment\n"
				   "password=abc#123\n"
				   "path=C:\\tmp;cache\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("http://example.test/a#fragment"), ini["main"].toString("url"));
		CHECK_EQ(std::string("abc#123"), ini["main"].toString("password"));
		CHECK_EQ(std::string("C:\\tmp;cache"), ini["main"].toString("path"));

		std::remove(path.c_str());
	}

	void test_quoted_comment_markers_are_preserved()
	{
		const std::string path = temp_path("quoted_markers");
		write_file(path, "[main]\ntext=\"hello ; world\"\nhash='a # b'\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("\"hello ; world\""), ini["main"].toString("text"));
		CHECK_EQ(std::string("'a # b'"), ini["main"].toString("hash"));

		std::remove(path.c_str());
	}

	void test_escaped_comment_markers_are_preserved()
	{
		const std::string path = temp_path("escaped_markers");
		write_file(path, "[main]\ntext=hello \\; world\nhash=hello \\# world\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("hello \\; world"), ini["main"].toString("text"));
		CHECK_EQ(std::string("hello \\# world"), ini["main"].toString("hash"));

		std::remove(path.c_str());
	}

	void test_colon_delimiter_is_supported_when_equals_is_absent()
	{
		const std::string path = temp_path("colon_delimiter");
		write_file(path, "[main]\nhost: localhost\nurl=http://example.test:8080/path\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("localhost"), ini["main"].toString("host"));
		CHECK_EQ(std::string("http://example.test:8080/path"), ini["main"].toString("url"));

		std::remove(path.c_str());
	}

	void test_empty_values_are_parsed()
	{
		const std::string path = temp_path("empty_values");
		write_file(path, "[main]\nempty=\nspace =    \n");

		inicpp::IniManager ini(path);
		std::map<std::string, std::string> values = ini.sectionMap("main");
		CHECK_TRUE(values.find("empty") != values.end());
		CHECK_TRUE(values.find("space") != values.end());
		CHECK_EQ(std::string(""), values["empty"]);
		CHECK_EQ(std::string(""), values["space"]);

		std::remove(path.c_str());
	}

	void test_bom_on_first_line_is_ignored()
	{
		const std::string path = temp_path("bom");
		write_file(path, std::string("\xEF\xBB\xBF") + "[main]\nkey=value\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("value"), ini["main"].toString("key"));

		std::remove(path.c_str());
	}

	void test_malformed_lines_are_ignored_without_throwing()
	{
		const std::string path = temp_path("malformed");
		write_file(path, "[main]\n[broken\nno_delimiter\ngood=value\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("value"), ini["main"].toString("good"));
		CHECK_TRUE(!ini["main"].isKeyExist("no_delimiter"));

		std::remove(path.c_str());
	}
}

int main()
{
	test_leading_whitespace_comments_are_ignored();
	test_section_header_allows_leading_and_inner_spaces();
	test_section_header_allows_inline_comment();
	test_inline_comment_is_stripped_when_preceded_by_space();
	test_inline_comment_markers_inside_values_are_preserved();
	test_quoted_comment_markers_are_preserved();
	test_escaped_comment_markers_are_preserved();
	test_colon_delimiter_is_supported_when_equals_is_absent();
	test_empty_values_are_parsed();
	test_bom_on_first_line_is_ignored();
	test_malformed_lines_are_ignored_without_throwing();
	std::cout << "inicpp_ini_format: PASS" << std::endl;
	return 0;
}
