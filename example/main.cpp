#include "inicpp.hpp"

#include <iostream>
#include <iomanip>

// ANSI escape code for green text
#define GREEN_TEXT "\033[1;32m"
#define RESET_COLOR "\033[0m"

#define CONFIG_FILE "config.ini"

void writeExample();
void readExample();
void sectionsList();

namespace Configer
{
	typedef struct Server
	{
		static const Server Load(inicpp::IniManager &_ini)
		{
			return Server{
				_ini["server"]["ip"],
				_ini["server"]["port"],
				_ini["server"]["keepalived"]
			};
		}

		std::string ip;
		unsigned short port;
		bool isKeepalived;
	} Server;

	typedef struct Config
	{
		static const Config Load()
		{
			inicpp::IniManager _ini(CONFIG_FILE);

			return Config{
				_ini[""]["title"],
				Server::Load(_ini),
				_ini["math"]["PI"],
			};
		}

		std::string title;
		Server server;
		double PI;
	} Config;

	const Config Load()
	{
		return Config::Load();
	}
}

/** compile: g++ -I../ -std=c++11 main.cpp -o iniExample */
int main()
{

	/** write and read example */
	writeExample();
	readExample();
	sectionsList();

	/** Bind to Struct */
	Configer::Config config = Configer::Load();
	// use 'config' you want
	// ...

	return 0;
}

void writeExample()
{
	inicpp::IniManager _ini(CONFIG_FILE);

	/** write to file. */
	// assignment
	_ini["server"]["number"] = 1;
	_ini["server"]["info"] = "the server socket info.";
	// or function
	_ini.set("server", "keepalived", "true");
	_ini.set("server", "port", 8080);
	_ini.set("server", "ip", "127.0.0.1");

	// set with comment
	_ini.set("math", "PI", "3.141592653589793238462643383279502884", "Comment: This is pi in mathematics."); // with a comment.
	_ini.setComment("server", "port", "this is the listen ip for server.");									 // comment to section->key

	// none section
	_ini.set("title", "config.ini");
	_ini.setComment("title", "This is the title."); // add comment for none section key
}

/**
 * Note: The functions toInt(), toDouble(), and toString() will never throw an exception.
 *       However, '_ini["any"]["any"]' may throw an error if a key is not found
 *       or if a conversion error occurs.
 */
void readExample()
{
	inicpp::IniManager _ini(CONFIG_FILE);

	std::string server_port_s = _ini["server"]["port"];
	std::string server_ip_s = _ini["server"]["ip"];

	int server_port_i = _ini["server"]["port"];
	// - Convert to string.
	std::string PI_s = _ini["math"]["PI"];
	PI_s = _ini["math"]["PI"].operator std::string();
	PI_s = _ini["math"]["PI"].String();
	PI_s = _ini["math"]["PI"].get<std::string>();
	// - Convert to double.
	double PI_d = _ini["math"]["PI"];
	// - Convert to int.
	int PI_i = _ini["math"]["PI"];
	unsigned int PI_ui = _ini["math"]["PI"];
	// - Or others
	float PI_f = _ini["math"]["PI"];
	short PI_si = _ini["math"]["PI"];
	char PI_c = _ini["math"]["PI"];
	// - DIY, any type you want
	long long PI_ll = _ini["math"]["PI"].get<long long>();

	PI_c = _ini["math"]["PI"];

	// - Not throw exceptions
	PI_s = _ini["math"].toString("PI");
	PI_d = _ini["math"].toDouble("PI");
	PI_i = _ini["math"].toInt("PI");

	// - Bind to Struct
	Configer::Config config = Configer::Load();
}

void sectionsList()
{
	inicpp::IniManager _ini;

	// set config file and parse
	_ini.setFileName(CONFIG_FILE);
	_ini.parse();

	for (auto &sectionName : _ini.sectionsList())
	{
		std::cout << GREEN_TEXT << "\nSection: " << RESET_COLOR << sectionName << "\n";

		for (auto &kv : _ini.sectionMap(sectionName))
		{
			std::cout << std::setw(10) << std::left << kv.first
					  << " -------> "
					  << kv.second << std::endl;
		}
	}
}
