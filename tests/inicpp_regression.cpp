#include "../inicpp.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
	void fail(const std::string &message, const char *file, int line)
	{
		std::cerr << file << ":" << line << ": " << message << std::endl;
		std::exit(1);
	}

#define CHECK_TRUE(expr)                                                                 \
	do                                                                                   \
	{                                                                                    \
		if (!(expr))                                                                     \
		{                                                                                \
			fail(std::string("CHECK_TRUE failed: ") + #expr, __FILE__, __LINE__);       \
		}                                                                                \
	} while (false)

#define CHECK_EQ(expected, actual)                                                       \
	do                                                                                   \
	{                                                                                    \
		if (!((expected) == (actual)))                                                    \
		{                                                                                \
			std::ostringstream oss;                                                      \
			oss << "CHECK_EQ failed: expected [" << (expected) << "], actual ["         \
				<< (actual) << "]";                                                     \
			fail(oss.str(), __FILE__, __LINE__);                                        \
		}                                                                                \
	} while (false)

	std::string temp_path(const std::string &name)
	{
		static std::random_device random;
		static unsigned long counter = 0;

		for (int attempt = 0; attempt < 16; ++attempt)
		{
			std::ostringstream oss;
			oss << "inicpp_" << name << "_" << random() << "_" << random() << "_" << counter++ << ".ini";
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

	std::string read_file(const std::string &path)
	{
		std::ifstream input(path.c_str());
		if (!input.is_open())
		{
			fail(std::string("failed to open file for reading: ") + path, __FILE__, __LINE__);
		}
		std::ostringstream buffer;
		buffer << input.rdbuf();
		return buffer.str();
	}

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

	std::size_t count_substring(const std::string &text, const std::string &needle)
	{
		std::size_t count = 0;
		std::size_t pos = 0;
		while ((pos = text.find(needle, pos)) != std::string::npos)
		{
			++count;
			pos += needle.size();
		}
		return count;
	}

	void test_public_api_roundtrip()
	{
		const std::string path = temp_path("roundtrip");
		std::remove(path.c_str());

		inicpp::IniManager ini(path);
		ini["server"]["number"] = 1;
		ini["server"]["info"] = "the server socket info.";
		CHECK_TRUE(ini.set("server", "keepalived", "true"));
		CHECK_TRUE(ini.set("server", "port", 8080));
		CHECK_TRUE(ini.set("server", "ip", "127.0.0.1"));
		CHECK_TRUE(ini.set("math", "PI", "3.141592653589793238462643383279502884",
						   "Comment: This is pi in mathematics."));
		CHECK_TRUE(ini.setComment("server", "port", "this is the listen ip for server."));
		CHECK_TRUE(ini.set("title", "config.ini"));
		CHECK_TRUE(ini.setComment("title", "This is the title."));

		inicpp::IniManager loaded(path);
		CHECK_EQ(std::string("127.0.0.1"), std::string(loaded["server"]["ip"]));
		CHECK_EQ(8080, int(loaded["server"]["port"]));
		CHECK_EQ(1, int(loaded["server"]["number"]));
		CHECK_TRUE(bool(loaded["server"]["keepalived"]));
		CHECK_EQ(std::string("config.ini"), std::string(loaded[""]["title"]));
		CHECK_EQ(std::string("8080"), loaded["server"].toString("port"));
		CHECK_EQ(8080, loaded["server"].toInt("port"));
		CHECK_TRUE(std::fabs(loaded["math"].toDouble("PI") - 3.14159265358979323846) < 0.000001);

		std::map<std::string, std::string> server = loaded.sectionMap("server");
		CHECK_EQ(std::string("127.0.0.1"), server["ip"]);
		CHECK_EQ(std::string("8080"), server["port"]);

		std::list<std::string> sections = loaded.sectionsList();
		CHECK_TRUE(contains_section(sections, ""));
		CHECK_TRUE(contains_section(sections, "server"));
		CHECK_TRUE(contains_section(sections, "math"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find(";Comment: This is pi in mathematics.") != std::string::npos);
		CHECK_TRUE(file.find(";this is the listen ip for server.") != std::string::npos);
		CHECK_TRUE(file.find(";This is the title.") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_conversion_errors()
	{
		const std::string path = temp_path("conversion");
		write_file(path, "[bad]\nnumber=abc\ntruth=false\nmissing=\n");

		inicpp::IniManager ini(path);
		CHECK_EQ(0, ini["bad"].toInt("number"));
		CHECK_TRUE(std::fabs(ini["bad"].toDouble("number")) < 0.000001);
		CHECK_EQ(std::string(""), ini["bad"].toString("absent"));
		CHECK_TRUE(!bool(ini["bad"]["truth"]));

		bool threw = false;
		try
		{
			(void)ini["bad"]["number"].get<int>();
		}
		catch (const std::runtime_error &)
		{
			threw = true;
		}
		CHECK_TRUE(threw);

		std::remove(path.c_str());
	}

	void test_update_replaces_single_key()
	{
		const std::string path = temp_path("replace");
		write_file(path, "[server]\n;old port\nport=8080\nip=127.0.0.1\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "port", "9090", "new port"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(9090, int(loaded["server"]["port"]));

		const std::string file = read_file(path);
		CHECK_EQ(static_cast<std::size_t>(1), count_substring(file, "port="));
		CHECK_TRUE(file.find(";new port") != std::string::npos);
		CHECK_TRUE(file.find(";old port") == std::string::npos);

		std::remove(path.c_str());
	}

	void test_missing_string_conversion_throws()
	{
		const std::string path = temp_path("missing_string");
		write_file(path, "");

		inicpp::IniManager ini(path);

		bool threw = false;
		try
		{
			std::string value = ini["System"]["LogLevel"];
			(void)value;
		}
		catch (const std::runtime_error &)
		{
			threw = true;
		}

		CHECK_TRUE(threw);
		std::remove(path.c_str());
	}

	void test_missing_get_string_throws()
	{
		const std::string path = temp_path("missing_get_string");
		write_file(path, "");

		inicpp::IniManager ini(path);

		bool threw = false;
		try
		{
			std::string value = ini["System"]["LogLevel"].get<std::string>();
			(void)value;
		}
		catch (const std::runtime_error &)
		{
			threw = true;
		}

		CHECK_TRUE(threw);
		std::remove(path.c_str());
	}

	void test_missing_bool_conversion_throws()
	{
		const std::string path = temp_path("missing_bool");
		write_file(path, "");

		inicpp::IniManager ini(path);

		bool threw = false;
		try
		{
			bool value = ini["System"]["enabled"];
			(void)value;
		}
		catch (const std::runtime_error &)
		{
			threw = true;
		}

		CHECK_TRUE(threw);
		std::remove(path.c_str());
	}

	void test_existing_empty_string_value_does_not_throw()
	{
		const std::string path = temp_path("existing_empty_string");
		write_file(path, "[System]\nLogLevel=\n");

		inicpp::IniManager ini(path);

		std::string value = ini["System"]["LogLevel"];
		CHECK_EQ(std::string(""), value);
		CHECK_EQ(std::string(""), ini["System"]["LogLevel"].get<std::string>());
		CHECK_EQ(std::string(""), ini["System"]["LogLevel"].String());

		std::remove(path.c_str());
	}

	void test_missing_lookup_does_not_create_section_in_memory()
	{
		const std::string path = temp_path("missing_lookup_no_mutation");
		write_file(path, "");

		inicpp::IniManager ini(path);

		try
		{
			std::string value = ini["System"]["LogLevel"];
			(void)value;
		}
		catch (const std::runtime_error &)
		{
		}

		CHECK_TRUE(!ini.isSectionExists("System"));
		std::remove(path.c_str());
	}
}

int main()
{
	test_public_api_roundtrip();
	test_conversion_errors();
	test_update_replaces_single_key();
	test_missing_string_conversion_throws();
	test_missing_get_string_throws();
	test_missing_bool_conversion_throws();
	test_existing_empty_string_value_does_not_throw();
	test_missing_lookup_does_not_create_section_in_memory();
	std::cout << "inicpp_regression: PASS" << std::endl;
	return 0;
}
