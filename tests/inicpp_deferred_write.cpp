#include "../inicpp.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
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
			oss << "inicpp_deferred_write_" << name << "_" << random() << "_"
				<< random() << "_" << counter++ << ".ini";
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

	void write_file_binary(const std::string &path, const std::string &content)
	{
		std::ofstream output(path.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
		CHECK_TRUE(output.is_open());
		output << content;
		output.close();
		CHECK_TRUE(static_cast<bool>(output));
	}

	std::string read_file(const std::string &path)
	{
		std::ifstream input(path.c_str());
		CHECK_TRUE(input.is_open());
		std::ostringstream buffer;
		buffer << input.rdbuf();
		return buffer.str();
	}

	std::string read_file_binary(const std::string &path)
	{
		std::ifstream input(path.c_str(), std::ifstream::in | std::ifstream::binary);
		CHECK_TRUE(input.is_open());
		std::ostringstream buffer;
		buffer << input.rdbuf();
		return buffer.str();
	}

	bool has_lone_lf(const std::string &content)
	{
		for (std::string::size_type i = 0; i < content.size(); ++i)
		{
			if (content[i] == '\n' && (i == 0 || content[i - 1] != '\r'))
			{
				return true;
			}
		}
		return false;
	}

	void test_deferred_set_updates_memory_without_writing_until_flush()
	{
		const std::string path = temp_path("memory_then_flush");
		const std::string original = "[server]\nport=8080\n[client]\nname=web\n";
		write_file(path, original);

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.isAutoFlushEnabled());
		CHECK_TRUE(!ini.isDirty());
		CHECK_TRUE(ini.setAutoFlush(false));
		CHECK_TRUE(!ini.isAutoFlushEnabled());

		CHECK_TRUE(ini.set("server", "port", "9090"));
		CHECK_TRUE(ini.set("server", "ip", "127.0.0.1"));
		CHECK_TRUE(ini.set("database", "host", "db.local"));

		CHECK_TRUE(ini.isDirty());
		CHECK_EQ(9090, int(ini["server"]["port"]));
		CHECK_EQ(std::string("127.0.0.1"), ini["server"].toString("ip"));
		CHECK_EQ(std::string("db.local"), ini["database"].toString("host"));
		CHECK_EQ(original, read_file(path));

		CHECK_TRUE(ini.flush());
		CHECK_TRUE(!ini.isDirty());
		CHECK_TRUE(!ini.isAutoFlushEnabled());

		inicpp::IniManager loaded(path);
		CHECK_EQ(9090, int(loaded["server"]["port"]));
		CHECK_EQ(std::string("127.0.0.1"), loaded["server"].toString("ip"));
		CHECK_EQ(std::string("db.local"), loaded["database"].toString("host"));

		std::remove(path.c_str());
	}

	void test_value_proxy_assignment_respects_deferred_flush()
	{
		const std::string path = temp_path("value_proxy");
		const std::string original = "[server]\nport=8080\n";
		write_file(path, original);

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setAutoFlush(false));

		ini["server"]["port"] = 9090;
		ini["server"]["name"] = std::string("api");

		CHECK_TRUE(ini.isDirty());
		CHECK_EQ(9090, int(ini["server"]["port"]));
		CHECK_EQ(std::string("api"), ini["server"].toString("name"));
		CHECK_EQ(original, read_file(path));

		CHECK_TRUE(ini.flush());
		CHECK_TRUE(!ini.isDirty());

		inicpp::IniManager loaded(path);
		CHECK_EQ(9090, int(loaded["server"]["port"]));
		CHECK_EQ(std::string("api"), loaded["server"].toString("name"));

		std::remove(path.c_str());
	}

	void test_deferred_parse_discards_unflushed_changes()
	{
		const std::string path = temp_path("parse_discards");
		const std::string original = "[server]\nport=8080\n";
		write_file(path, original);

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setAutoFlush(false));
		CHECK_TRUE(ini.set("server", "port", "9090"));
		CHECK_TRUE(ini.isDirty());
		CHECK_EQ(9090, int(ini["server"]["port"]));

		ini.parse();

		CHECK_TRUE(!ini.isDirty());
		CHECK_EQ(8080, int(ini["server"]["port"]));
		CHECK_EQ(original, read_file(path));

		std::remove(path.c_str());
	}

	void test_deferred_flush_preserves_line_endings_and_comments()
	{
		const std::string path = temp_path("crlf_comment");
		write_file_binary(path, "[server]\r\nport=8080\r\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setAutoFlush(false));
		CHECK_TRUE(ini.set("server", "port", "9090", "new port"));
		CHECK_TRUE(ini.flush());

		const std::string file = read_file_binary(path);
		CHECK_TRUE(file.find("[server]\r\n;new port\r\nport=9090\r\n") != std::string::npos);
		CHECK_TRUE(!has_lone_lf(file));

		std::remove(path.c_str());
	}

	void test_reenabling_auto_flush_flushes_pending_changes()
	{
		const std::string path = temp_path("reenable_auto_flush");
		write_file(path, "[server]\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setAutoFlush(false));
		CHECK_TRUE(ini.set("server", "port", "9090"));
		CHECK_TRUE(ini.isDirty());

		CHECK_TRUE(ini.setAutoFlush(true));
		CHECK_TRUE(ini.isAutoFlushEnabled());
		CHECK_TRUE(!ini.isDirty());
		CHECK_EQ(9090, int(inicpp::IniManager(path)["server"]["port"]));

		CHECK_TRUE(ini.set("server", "port", "10000"));
		CHECK_EQ(10000, int(inicpp::IniManager(path)["server"]["port"]));

		std::remove(path.c_str());
	}
}

int main()
{
	test_deferred_set_updates_memory_without_writing_until_flush();
	test_value_proxy_assignment_respects_deferred_flush();
	test_deferred_parse_discards_unflushed_changes();
	test_deferred_flush_preserves_line_endings_and_comments();
	test_reenabling_auto_flush_flushes_pending_changes();
	std::cout << "inicpp_deferred_write: PASS" << std::endl;
	return 0;
}
