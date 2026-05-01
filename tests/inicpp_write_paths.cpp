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
			oss << "inicpp_write_paths_" << name << "_" << random() << "_" << random()
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

	void test_appends_key_to_existing_section_before_next_section()
	{
		const std::string path = temp_path("append_existing");
		write_file(path, "[server]\nport=8080\n[client]\nport=7070\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "host", "localhost"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(std::string("localhost"), loaded["server"].toString("host"));
		CHECK_EQ(7070, int(loaded["client"]["port"]));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find("host=localhost") < file.find("[client]"));

		std::remove(path.c_str());
	}

	void test_creates_missing_section_at_end()
	{
		const std::string path = temp_path("missing_section");
		write_file(path, "[server]\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("database", "host", "localhost"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(std::string("localhost"), loaded["database"].toString("host"));
		CHECK_TRUE(read_file(path).find("[database]\nhost=localhost") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_sets_unnamed_key_at_file_head()
	{
		const std::string path = temp_path("unnamed_head");
		write_file(path, "[server]\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("title", "config.ini"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(std::string("config.ini"), loaded[""].toString("title"));
		CHECK_TRUE(read_file(path).find("title=config.ini\n[server]") == 0);

		std::remove(path.c_str());
	}

	void test_updates_only_target_section_when_keys_repeat()
	{
		const std::string path = temp_path("repeat_key_sections");
		write_file(path, "[server]\nport=8080\n[client]\nport=7070\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("client", "port", "9090"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(8080, int(loaded["server"]["port"]));
		CHECK_EQ(9090, int(loaded["client"]["port"]));

		std::remove(path.c_str());
	}

	void test_appends_key_to_empty_section()
	{
		const std::string path = temp_path("empty_section");
		write_file(path, "[empty]\n[other]\nkey=value\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("empty", "created", "yes"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(std::string("yes"), loaded["empty"].toString("created"));
		CHECK_TRUE(read_file(path).find("created=yes\n[other]") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_rejects_empty_key_without_changing_file()
	{
		const std::string path = temp_path("invalid_input");
		const std::string original = "[server]\nport=8080\n";
		write_file(path, original);

		inicpp::IniManager ini(path);
		CHECK_TRUE(!ini.set("server", "   ", "value"));
		CHECK_EQ(original, read_file(path));

		std::remove(path.c_str());
	}

	void test_set_writes_empty_value()
	{
		const std::string path = temp_path("empty_value");
		write_file(path, "[server]\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "empty", ""));

		inicpp::IniManager loaded(path);
		CHECK_TRUE(loaded["server"].isKeyExists("empty"));
		CHECK_EQ(std::string(""), loaded["server"].toString("empty"));
		CHECK_TRUE(read_file(path).find("empty=\n") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_value_proxy_assignment_writes_existing_key_to_empty_value()
	{
		const std::string path = temp_path("proxy_empty_value");
		write_file(path, "[server]\nnote=present\n");

		inicpp::IniManager ini(path);
		ini["server"]["note"] = std::string("");

		inicpp::IniManager loaded(path);
		CHECK_TRUE(loaded["server"].isKeyExists("note"));
		CHECK_EQ(std::string(""), loaded["server"].toString("note"));
		CHECK_TRUE(read_file(path).find("note=\n") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_comment_with_existing_semicolon_is_not_double_prefixed()
	{
		const std::string path = temp_path("semicolon_comment");
		write_file(path, "[server]\nkey=old\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "key", "new", ";already commented"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find(";already commented\nkey=new") != std::string::npos);
		CHECK_TRUE(file.find(";;already commented") == std::string::npos);

		std::remove(path.c_str());
	}

	void test_comment_with_existing_hash_is_not_semicolon_prefixed()
	{
		const std::string path = temp_path("hash_comment");
		write_file(path, "[server]\nkey=old\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "key", "new", "#already commented"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find("#already commented\nkey=new") != std::string::npos);
		CHECK_TRUE(file.find(";#already commented") == std::string::npos);

		std::remove(path.c_str());
	}

	void test_set_replaces_leading_whitespace_hash_comment()
	{
		const std::string path = temp_path("replace_hash_comment");
		write_file(path, "[server]\n   # old port\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "port", "9090", "new port"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find("# old port") == std::string::npos);
		CHECK_TRUE(file.find(";new port\nport=9090\n") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_set_preserves_crlf_line_endings()
	{
		const std::string path = temp_path("crlf_write");
		write_file_binary(path, "[server]\r\nport=8080\r\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "host", "localhost"));

		const std::string file = read_file_binary(path);
		CHECK_TRUE(file.find("[server]\r\n") != std::string::npos);
		CHECK_TRUE(file.find("port=8080\r\nhost=localhost\r\n") != std::string::npos);
		CHECK_TRUE(!has_lone_lf(file));

		std::remove(path.c_str());
	}

	void test_set_comment_creates_missing_section_key_with_empty_value()
	{
		const std::string path = temp_path("comment_missing_section_key");
		write_file(path, "[server]\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setComment("database", "host", "database host"));

		inicpp::IniManager loaded(path);
		CHECK_TRUE(loaded.isSectionExists("database"));
		CHECK_TRUE(loaded["database"].isKeyExist("host"));
		CHECK_EQ(std::string(""), loaded["database"].toString("host"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find("[database]\n;database host\nhost=") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_set_comment_creates_missing_unnamed_key_with_empty_value()
	{
		const std::string path = temp_path("comment_missing_unnamed_key");
		write_file(path, "[server]\nport=8080\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setComment("title", "configuration title"));

		inicpp::IniManager loaded(path);
		CHECK_TRUE(loaded[""].isKeyExist("title"));
		CHECK_EQ(std::string(""), loaded[""].toString("title"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find(";configuration title\ntitle=\n[server]") == 0);

		std::remove(path.c_str());
	}

	void test_set_comment_updates_existing_empty_value_key()
	{
		const std::string path = temp_path("comment_existing_empty_value");
		write_file(path, "[server]\nport=\n");

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.setComment("server", "port", "server port"));

		inicpp::IniManager loaded(path);
		CHECK_TRUE(loaded["server"].isKeyExist("port"));
		CHECK_EQ(std::string(""), loaded["server"].toString("port"));

		const std::string file = read_file(path);
		CHECK_TRUE(file.find("[server]\n;server port\nport=\n") != std::string::npos);

		std::remove(path.c_str());
	}

	void test_set_on_new_file_creates_ini_file()
	{
		const std::string path = temp_path("new_file");
		std::remove(path.c_str());

		inicpp::IniManager ini(path);
		CHECK_TRUE(ini.set("server", "port", "8080"));

		inicpp::IniManager loaded(path);
		CHECK_EQ(8080, int(loaded["server"]["port"]));

		std::remove(path.c_str());
	}
}

int main()
{
	test_appends_key_to_existing_section_before_next_section();
	test_creates_missing_section_at_end();
	test_sets_unnamed_key_at_file_head();
	test_updates_only_target_section_when_keys_repeat();
	test_appends_key_to_empty_section();
	test_rejects_empty_key_without_changing_file();
	test_set_writes_empty_value();
	test_value_proxy_assignment_writes_existing_key_to_empty_value();
	test_comment_with_existing_semicolon_is_not_double_prefixed();
	test_comment_with_existing_hash_is_not_semicolon_prefixed();
	test_set_replaces_leading_whitespace_hash_comment();
	test_set_preserves_crlf_line_endings();
	test_set_comment_creates_missing_section_key_with_empty_value();
	test_set_comment_creates_missing_unnamed_key_with_empty_value();
	test_set_comment_updates_existing_empty_value_key();
	test_set_on_new_file_creates_ini_file();
	std::cout << "inicpp_write_paths: PASS" << std::endl;
	return 0;
}
