#if defined(_WIN32)
#include <iostream>
int main()
{
	std::cout << "inicpp_file_safety: SKIP on Windows" << std::endl;
	return 0;
}
#else
#include "../inicpp.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
		return dir + "/" + name;
	}
}

int main()
{
	const std::string base = std::string("/tmp/inicpp_file_safety_") + std::to_string(static_cast<long long>(getpid()));
	const std::string targetDir = base + "_target";
	const std::string cwdDir = base + "_cwd";
	const std::string configPath = make_path(targetDir, "config.ini");

	rmdir(targetDir.c_str());
	rmdir(cwdDir.c_str());
	check(mkdir(targetDir.c_str(), 0700) == 0, "failed to create target dir");
	check(mkdir(cwdDir.c_str(), 0700) == 0, "failed to create cwd dir");
	write_file(configPath, "[server]\nport=8080\n");

	char oldCwd[4096];
	check(getcwd(oldCwd, sizeof(oldCwd)) != NULL, "failed to capture cwd");
	check(chmod(cwdDir.c_str(), 0555) == 0, "failed to make cwd read-only");
	check(chdir(cwdDir.c_str()) == 0, "failed to chdir into read-only cwd");

	bool ok = false;
	{
		inicpp::IniManager ini(configPath);
		ok = ini.set("server", "port", "9090");
	}

	check(chdir(oldCwd) == 0, "failed to restore cwd");
	chmod(cwdDir.c_str(), 0700);

	check(ok, "set() should not require writable process cwd");
	inicpp::IniManager loaded(configPath);
	check(int(loaded["server"]["port"]) == 9090, "updated port was not persisted");
	check(read_file(configPath).find("port=9090") != std::string::npos, "file does not contain updated port");

	std::remove(configPath.c_str());
	rmdir(targetDir.c_str());
	rmdir(cwdDir.c_str());

	std::cout << "inicpp_file_safety: PASS" << std::endl;
	return 0;
}
#endif
