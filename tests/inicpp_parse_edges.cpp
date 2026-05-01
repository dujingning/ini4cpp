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

	std::string temp_path(const std::string &name)
	{
		static std::random_device random;
		static unsigned long counter = 0;

		for (int attempt = 0; attempt < 16; ++attempt)
		{
			std::ostringstream oss;
			oss << "inicpp_parse_edges_" << name << "_" << random() << "_" << random()
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

	bool file_exists(const std::string &path)
	{
		std::ifstream input(path.c_str());
		return input.good();
	}

	void test_crlf_and_whitespace_only_lines()
	{
		const std::string path = temp_path("crlf");
		write_file(path, "   \r\n[main]\r\nkey = value\r\n\t\r\nother: data\r\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("value"), ini["main"].toString("key"));
		CHECK_EQ(std::string("data"), ini["main"].toString("other"));

		std::remove(path.c_str());
	}

	void test_invalid_section_header_tail_is_not_section()
	{
		const std::string path = temp_path("invalid_section_tail");
		write_file(path, "[main] trailing\nkey=value\n[valid]\nkey=ok\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(!ini.isSectionExists("main"));
		CHECK_EQ(std::string("value"), ini[""].toString("key"));
		CHECK_EQ(std::string("ok"), ini["valid"].toString("key"));

		std::remove(path.c_str());
	}

	void test_empty_section_header_is_ignored()
	{
		const std::string path = temp_path("empty_section_header");
		write_file(path, "[]\nkey=value\n[valid]\nkey=ok\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("value"), ini[""].toString("key"));
		CHECK_EQ(std::string("ok"), ini["valid"].toString("key"));

		std::remove(path.c_str());
	}

	void test_empty_keys_are_skipped()
	{
		const std::string path = temp_path("empty_keys");
		write_file(path, "[main]\n=value\n : value\nvalid=yes\n");

		inicpp::IniManager ini(path);
		std::map<std::string, std::string> values = ini.sectionMap("main");
		CHECK_TRUE(values.find("") == values.end());
		CHECK_EQ(std::string("yes"), values["valid"]);

		std::remove(path.c_str());
	}

	void test_multiple_delimiters_keep_remainder_in_value()
	{
		const std::string path = temp_path("multiple_delimiters");
		write_file(path, "[main]\nexpr=a=b=c\nurl: http://example.test:8080/path\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("a=b=c"), ini["main"].toString("expr"));
		CHECK_EQ(std::string("http://example.test:8080/path"), ini["main"].toString("url"));

		std::remove(path.c_str());
	}

	void test_unmatched_quote_preserves_comment_marker()
	{
		const std::string path = temp_path("unmatched_quote");
		write_file(path, "[main]\ntext=\"hello ; still value\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("\"hello ; still value"), ini["main"].toString("text"));

		std::remove(path.c_str());
	}

	void test_duplicate_key_in_same_section_uses_last_value()
	{
		const std::string path = temp_path("duplicate_key");
		write_file(path, "[main]\nkey=first\nkey=second\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("second"), ini["main"].toString("key"));
		CHECK_EQ(3, ini["main"].getLine("key"));

		std::remove(path.c_str());
	}

	void test_duplicate_section_merges_with_last_value()
	{
		const std::string path = temp_path("duplicate_section");
		write_file(path, "[main]\nshared=first\nfirst_only=yes\n[main]\nshared=second\nsecond_only=yes\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(std::string("second"), ini["main"].toString("shared"));
		CHECK_EQ(std::string("yes"), ini["main"].toString("first_only"));
		CHECK_EQ(std::string("yes"), ini["main"].toString("second_only"));

		std::remove(path.c_str());
	}

	void test_constructing_missing_file_does_not_create_it()
	{
		const std::string path = temp_path("missing_read");
		std::remove(path.c_str());

		{
			inicpp::IniManager ini(path);
			CHECK_TRUE(!ini.isSectionExists("anything"));
		}

		CHECK_TRUE(!file_exists(path));
	}

	void test_parse_replaces_previous_state()
	{
		const std::string path = temp_path("parse_replace");
		write_file(path, "[first]\nkey=value\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.isSectionExists("first"));

		write_file(path, "[second]\nkey=value\n");
		ini.parse();

		CHECK_TRUE(!ini.isSectionExists("first"));
		CHECK_TRUE(ini.isSectionExists("second"));
		CHECK_EQ(std::string("value"), ini["second"].toString("key"));

		std::remove(path.c_str());
	}
}

int main()
{
	test_crlf_and_whitespace_only_lines();
	test_invalid_section_header_tail_is_not_section();
	test_empty_section_header_is_ignored();
	test_empty_keys_are_skipped();
	test_multiple_delimiters_keep_remainder_in_value();
	test_unmatched_quote_preserves_comment_marker();
	test_duplicate_key_in_same_section_uses_last_value();
	test_duplicate_section_merges_with_last_value();
	test_constructing_missing_file_does_not_create_it();
	test_parse_replaces_previous_state();
	std::cout << "inicpp_parse_edges: PASS" << std::endl;
	return 0;
}
