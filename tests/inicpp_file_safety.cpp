#include "../inicpp.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace
{
	void check(bool condition, const std::string &message)
	{
		if (!condition)
		{
			std::cerr << message << std::endl;
			std::exit(1);
		}
	}

	std::string read_file(const std::string &path)
	{
		std::ifstream input(path.c_str());
		std::ostringstream buffer;
		buffer << input.rdbuf();
		return buffer.str();
	}

	char path_separator()
	{
#if defined(_WIN32)
		return '\\';
#else
		return '/';
#endif
	}

	void write_file(const std::string &path, const std::string &content)
	{
		std::ofstream output(path.c_str(), std::ofstream::out | std::ofstream::trunc);
		check(output.is_open(), "failed to open output file");
		output << content;
		output.close();
		check(static_cast<bool>(output), "failed to write output file");
	}

	std::string make_path(const std::string &dir, const std::string &name)
	{
		return dir + path_separator() + name;
	}

	std::string temp_base_path()
	{
#if defined(_WIN32)
		char tempPath[MAX_PATH + 1];
		DWORD length = GetTempPathA(MAX_PATH, tempPath);
		check(length > 0 && length <= MAX_PATH, "failed to get temp path");
		std::string base(tempPath);
		if (!base.empty() && base[base.size() - 1] != '\\' && base[base.size() - 1] != '/')
		{
			base += path_separator();
		}
		return base + "inicpp_file_safety_" + std::to_string(static_cast<long long>(_getpid()));
#else
		return std::string("/tmp/inicpp_file_safety_") + std::to_string(static_cast<long long>(getpid()));
#endif
	}

	bool make_dir(const std::string &path)
	{
#if defined(_WIN32)
		return _mkdir(path.c_str()) == 0;
#else
		return mkdir(path.c_str(), 0700) == 0;
#endif
	}

	void remove_dir(const std::string &path)
	{
#if defined(_WIN32)
		_rmdir(path.c_str());
#else
		rmdir(path.c_str());
#endif
	}

	bool change_dir(const std::string &path)
	{
#if defined(_WIN32)
		return _chdir(path.c_str()) == 0;
#else
		return chdir(path.c_str()) == 0;
#endif
	}

	std::string current_dir()
	{
		char buffer[4096];
#if defined(_WIN32)
		check(_getcwd(buffer, sizeof(buffer)) != NULL, "failed to capture cwd");
#else
		check(getcwd(buffer, sizeof(buffer)) != NULL, "failed to capture cwd");
#endif
		return buffer;
	}

	void make_cwd_read_only_if_supported(const std::string &path)
	{
#if defined(_WIN32)
		(void)path;
#else
		check(chmod(path.c_str(), 0555) == 0, "failed to make cwd read-only");
#endif
	}

	void restore_cwd_permissions_if_needed(const std::string &path)
	{
#if defined(_WIN32)
		(void)path;
#else
		chmod(path.c_str(), 0700);
#endif
	}
}

int main()
{
	const std::string base = temp_base_path();
	const std::string targetDir = base + "_target";
	const std::string cwdDir = base + "_cwd";
	const std::string configPath = make_path(targetDir, "config.ini");

	remove_dir(targetDir);
	remove_dir(cwdDir);
	check(make_dir(targetDir), "failed to create target dir");
	check(make_dir(cwdDir), "failed to create cwd dir");
	write_file(configPath, "[server]\nport=8080\n");

	const std::string oldCwd = current_dir();
	make_cwd_read_only_if_supported(cwdDir);
	check(change_dir(cwdDir), "failed to chdir into alternate cwd");

	bool ok = false;
	{
		inicpp::IniManager ini(configPath);
		ok = ini.set("server", "port", "9090");
	}

	check(change_dir(oldCwd), "failed to restore cwd");
	restore_cwd_permissions_if_needed(cwdDir);

	check(ok, "set() should not require process cwd to match target file directory");
	inicpp::IniManager loaded(configPath);
	check(int(loaded["server"]["port"]) == 9090, "updated port was not persisted");
	check(read_file(configPath).find("port=9090") != std::string::npos, "file does not contain updated port");

	std::remove(configPath.c_str());
	remove_dir(targetDir);
	remove_dir(cwdDir);

	std::cout << "inicpp_file_safety: PASS" << std::endl;
	return 0;
}
