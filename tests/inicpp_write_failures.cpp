#include <algorithm>
#include <cctype>
#include <cstddef>
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
#include <type_traits>

#if !defined(_WIN32)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace std
{
	int inicpp_test_rename(const char *oldName, const char *newName);
	int inicpp_test_remove(const char *path);
}

#define private public
#define rename inicpp_test_rename
#define remove inicpp_test_remove
#include "../inicpp.hpp"
#undef remove
#undef rename
#undef private

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

	enum RenameFailureMode
	{
		RENAME_NORMAL,
		FAIL_FIRST_RENAME,
		FAIL_SECOND_RENAME,
		FAIL_SECOND_AND_RESTORE_RENAME,
		FAIL_BACKUP_REMOVE
	};

	RenameFailureMode g_mode = RENAME_NORMAL;
	int g_renameCalls = 0;

	bool ends_with(const std::string &text, const std::string &suffix)
	{
		return text.size() >= suffix.size() &&
			   text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
	}

	void reset_file_ops(const RenameFailureMode mode)
	{
		g_mode = mode;
		g_renameCalls = 0;
	}

	std::string temp_path(const std::string &name)
	{
		static std::random_device random;
		static unsigned long counter = 0;

		for (int attempt = 0; attempt < 16; ++attempt)
		{
			std::ostringstream oss;
			oss << "inicpp_write_failures_" << name << "_" << random() << "_" << random()
				<< "_" << counter++;
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
		std::ifstream input(path.c_str(), std::ifstream::in | std::ifstream::binary);
		CHECK_TRUE(input.is_open());
		std::ostringstream buffer;
		buffer << input.rdbuf();
		return buffer.str();
	}

	bool file_exists(const std::string &path)
	{
		std::ifstream input(path.c_str(), std::ifstream::in | std::ifstream::binary);
		return input.good();
	}

	void remove_file(const std::string &path)
	{
		std::remove(path.c_str());
	}

	void cleanup_replace_files(const std::string &path)
	{
		remove_file(path + ".inicpp.tmp");
		remove_file(path + ".inicpp.bak");
		remove_file(path);
	}

	void test_rename_failure_removes_temp_file_and_keeps_original()
	{
		const std::string path = temp_path("rename_failure.ini");
		const std::string original = "[server]\nport=8080\n";
		cleanup_replace_files(path);
		write_file(path, original);

		reset_file_ops(FAIL_FIRST_RENAME);
		CHECK_TRUE(!inicpp::IniManager::replaceFileWithBackup(path, "[server]\nport=9090\n"));
		reset_file_ops(RENAME_NORMAL);

		CHECK_EQ(original, read_file(path));
		CHECK_TRUE(!file_exists(path + ".inicpp.tmp"));
		CHECK_TRUE(!file_exists(path + ".inicpp.bak"));

		cleanup_replace_files(path);
	}

	void test_temp_replace_failure_restores_backup_and_removes_temp()
	{
		const std::string path = temp_path("replace_failure.ini");
		const std::string original = "[server]\nport=8080\n";
		cleanup_replace_files(path);
		write_file(path, original);

		reset_file_ops(FAIL_SECOND_RENAME);
		CHECK_TRUE(!inicpp::IniManager::replaceFileWithBackup(path, "[server]\nport=9090\n"));
		reset_file_ops(RENAME_NORMAL);

		CHECK_EQ(original, read_file(path));
		CHECK_TRUE(!file_exists(path + ".inicpp.tmp"));
		CHECK_TRUE(!file_exists(path + ".inicpp.bak"));

		cleanup_replace_files(path);
	}

	void test_backup_restore_failure_leaves_backup_for_recovery()
	{
		const std::string path = temp_path("restore_failure.ini");
		const std::string original = "[server]\nport=8080\n";
		cleanup_replace_files(path);
		write_file(path, original);

		reset_file_ops(FAIL_SECOND_AND_RESTORE_RENAME);
		CHECK_TRUE(!inicpp::IniManager::replaceFileWithBackup(path, "[server]\nport=9090\n"));
		reset_file_ops(RENAME_NORMAL);

		CHECK_TRUE(!file_exists(path));
		CHECK_EQ(original, read_file(path + ".inicpp.bak"));
		CHECK_TRUE(!file_exists(path + ".inicpp.tmp"));

		std::rename((path + ".inicpp.bak").c_str(), path.c_str());
		cleanup_replace_files(path);
	}

	void test_backup_remove_failure_removes_temp_and_keeps_files()
	{
		const std::string path = temp_path("backup_remove_failure.ini");
		const std::string original = "[server]\nport=8080\n";
		const std::string backup = "[server]\nport=7070\n";
		cleanup_replace_files(path);
		write_file(path, original);
		write_file(path + ".inicpp.bak", backup);

		reset_file_ops(FAIL_BACKUP_REMOVE);
		CHECK_TRUE(!inicpp::IniManager::replaceFileWithBackup(path, "[server]\nport=9090\n"));
		reset_file_ops(RENAME_NORMAL);

		CHECK_EQ(original, read_file(path));
		CHECK_EQ(backup, read_file(path + ".inicpp.bak"));
		CHECK_TRUE(!file_exists(path + ".inicpp.tmp"));

		cleanup_replace_files(path);
	}

	bool can_create_file(const std::string &path)
	{
		std::ofstream output(path.c_str(), std::ofstream::out | std::ofstream::trunc);
		const bool ok = output.is_open();
		output.close();
		if (ok)
		{
			std::remove(path.c_str());
		}
		return ok;
	}

	void test_target_directory_without_write_permission_rejects_set()
	{
#if defined(_WIN32)
		std::cout << "inicpp_write_failures: skipping POSIX directory permission test on Windows" << std::endl;
#else
		const std::string dir = temp_path("readonly_dir");
		CHECK_TRUE(::mkdir(dir.c_str(), 0700) == 0);

		const std::string path = dir + "/config.ini";
		const std::string original = "[server]\nport=8080\n";
		write_file(path, original);

		CHECK_TRUE(::chmod(dir.c_str(), 0555) == 0);
		if (can_create_file(dir + "/probe"))
		{
			CHECK_TRUE(::chmod(dir.c_str(), 0700) == 0);
			remove_file(path);
			::rmdir(dir.c_str());
			return;
		}

		inicpp::IniManager ini(path);
		CHECK_TRUE(!ini.set("server", "port", "9090"));

		CHECK_TRUE(::chmod(dir.c_str(), 0700) == 0);
		CHECK_EQ(original, read_file(path));
		CHECK_TRUE(!file_exists(path + ".inicpp.tmp"));
		CHECK_TRUE(!file_exists(path + ".inicpp.bak"));

		remove_file(path);
		::rmdir(dir.c_str());
#endif
	}
}

namespace std
{
	int inicpp_test_rename(const char *oldName, const char *newName)
	{
		++g_renameCalls;
		if (g_mode == FAIL_FIRST_RENAME && g_renameCalls == 1)
		{
			return -1;
		}
		if (g_mode == FAIL_SECOND_RENAME && g_renameCalls == 2)
		{
			return -1;
		}
		if (g_mode == FAIL_SECOND_AND_RESTORE_RENAME && (g_renameCalls == 2 || g_renameCalls == 3))
		{
			return -1;
		}
		return ::rename(oldName, newName);
	}

	int inicpp_test_remove(const char *path)
	{
		if (g_mode == FAIL_BACKUP_REMOVE && ends_with(path, ".inicpp.bak"))
		{
			return -1;
		}
		return ::remove(path);
	}
}

int main()
{
	test_rename_failure_removes_temp_file_and_keeps_original();
	test_temp_replace_failure_restores_backup_and_removes_temp();
	test_backup_restore_failure_leaves_backup_for_recovery();
	test_backup_remove_failure_removes_temp_and_keeps_files();
	test_target_directory_without_write_permission_rejects_set();
	std::cout << "inicpp_write_failures: PASS" << std::endl;
	return 0;
}
