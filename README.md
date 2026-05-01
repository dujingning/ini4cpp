### What is this C++ library for?

> 🔥 1.Aims to make INI file processing **simple**, **convenient**, and **intuitive**.

> 🔥 2.Advancing the enduring excellence of **C++** through collaborative innovation.

> 🔥 3.Let C++ keep **simple**,**easy** and **free**.

---

### Ⅰ、Project
You can view the project at [https://github.com/dujingning/inicpp.git](https://github.com/dujingning/inicpp.git) or [https://gitee.com/dujingning/inicpp](https://gitee.com/dujingning/inicpp).

#### * To support open source, please give us a star. For any issues, feel free to open an issue. Thank you very much!


---

### Ⅱ、Description

The INI header-only library for Modern C++ supports **reading**, **writing**, and even **commenting**. It is easy to use and simplifies working with INI files.

- New Feature : [Super Easy Binding to Your Data Structures (For Read)](#7super-easy-binding-to-your-data-structures-read).

#### Supported INI syntax

When reading INI files, inicpp supports these common forms:

- UTF-8 BOM at the beginning of the file.
- Section headers such as `[server]`, `  [server]`, and `[ server ]`.
- Key-value lines using `=` or `:`, such as `port=8080` and `host: localhost`.
- Empty values in existing files and values written with `set(..., "")`, such as `key=`.
- Full-line comments using `;` or `#`, including lines with leading whitespace.
- Inline comments after whitespace, such as `key=value ; comment` and `key=value # comment`.
- Comment markers inside quoted or escaped values are preserved, such as `text="a ; b"` and `path=C:\tmp;cache`.
- Duplicate keys in the same section, and duplicate section headers, use the last parsed value for the repeated key.

Quoted values are returned as written; inicpp does not automatically remove quotes or unescape values. Write operations preserve the existing LF or CRLF line-ending style, but multiline values, bare keys without `=` or `:`, and full formatting-preserving round trips are not supported.


---

### Ⅲ、Usage 

#### * 0.Simple to use with C++11 or later.
```
git clone https://github.com/dujingning/inicpp.git
```

Include `inicpp.hpp`, declare the `inicpp::IniManager` class, and you're all set.


#### 1.write example
Write: Set directly to the file.
```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // Load and parse the INI file.

    _ini["server"]["ip"] = "192.168.3.35";
    _ini["server"]["port"] = 554;
    std::cout << _ini["server"]["ip"] << ":"<< _ini["server"]["port"] << std::endl;

    // or set any
    _ini.set("server","ip","127.0.0.1");
    _ini.set("server","port",8080);
    std::cout << _ini["server"]["ip"] << ":"<< _ini["server"]["port"] << std::endl;
}
```


#### 2.read example
Convert: From string to typed values. `operator[]` value conversions, including `std::string`, `bool`, and `get<T>()`, throw `std::runtime_error` when the section/key is missing or conversion fails. Existing empty values are valid and convert to an empty `std::string`; use `toString()`, `toInt()`, and `toDouble()` for non-throwing default-value reads. `bool` conversion treats only `"0"`, `"false"`, and `"no"` as false.
```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // Load and parse the INI file.

    int         port = _ini["server"]["port"];
    std::string ip   = _ini["server"]["ip"];
    std::cout << ip << ":"<< port << std::endl;

    // or get any
    ip   = _ini["server"]["ip"].get<std::string>();
    port = _ini["server"]["port"].get<int>();
    std::cout << ip << ":"<< port << std::endl;
}
```


#### 3.comment example
Comment: Write comments for key-value pairs.
```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // Load and parse the INI file.

    // comment section/key
    _ini.set("math"/*section*/, "PI"/*key*/, "3.1415926535897932"/*key*/, "This is PI in mathematics."/*comment*/);
    _ini.setComment("server"/*section*/, "port"/*key*/, "this is the listen ip for server."/*comment*/);
}
```

Comments written through `set()` or `setComment()` are prefixed with `;` by default. If the comment string already starts with `;` or `#`, that marker is kept.


#### 4.toString()、toInt()、toDouble()
Convert: From string to type without throwing. `toString()` returns an empty string for missing keys, while `toInt()` and `toDouble()` return `0` or `0.0` when the key is missing or conversion fails.
```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // Load and parse the INI file.
    
    _ini.set("server","port","554","this is the listen port for server");
    std::cout << _ini["server"]["port"] << std::endl;

    // toString
    std::string http_port_s = _ini["server"].toString("port");
    std::cout << "to string:\tserver.port = " << http_port_s << std::endl;

    // toDouble
    double http_port_d = _ini["server"].toDouble("port");
    std::cout << "to double:\tserver.port = " << http_port_d << std::endl;

    // toInt
    int http_port_i = _ini["server"].toInt("port");
    std::cout << "to int:\t\tserver.port = " << http_port_i << std::endl;
}
```


#### 5.isKeyExists()、sectionsList()、sectionMap()
May contain unnamed sections: when keys are at the head of the file.
```cpp
#include "inicpp.hpp"
#include <iostream>
#include <iomanip>

// ANSI escape code for green text
#define GREEN_TEXT "\033[1;32m"
#define RESET_COLOR "\033[0m"

int main()
{
    inicpp::IniManager _ini("config.ini"); // Load and parse the INI file.

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
```

#### 6.std::wstring
```cpp
#ifndef _ENBABLE_INICPP_STD_WSTRING_
#define _ENBABLE_INICPP_STD_WSTRING_ // std::wstring support
#endif

#include "inicpp.hpp"

#include <bits/stdc++.h>

int main()
{
    {
        inicpp::IniManager _ini(L"config.ini"); // Load and parse the INI file.
        std::wstring ws = _ini["server"].toWString("info");
        int port = _ini["server"]["port"];
        std::string ip = _ini["server"]["ip"];

        std::cout << ip << ":" << port << std::endl;
    }
    // or
    {
        inicpp::IniManager _ini;
        _ini.setFileName(L"config.ini");
        _ini.parse();
        std::wstring ws = _ini["server"].toWString("info");
        int port = _ini["server"]["port"];
        std::string ip = _ini["server"]["ip"];

        std::cout << ip << ":" << port << std::endl;
    }
}
```


#### 7.Super Easy Binding to Your Data Structures (Read).

config.ini :
```
title=config.ini
[server]
isKeepalived=true
;this is the listen ip for server.
port=8080
ip=127.0.0.1


[math]
;Comment: This is pi in mathematics.
PI=3.141592653589793238462643383279502884
```
config.cpp
```cpp
#include "inicpp.hpp"
#include <iostream>
#include <iomanip>

#define CONFIG_FILE "config.ini"

class app
{
public:
	typedef struct Config
	{
		typedef struct Server
		{
			std::string ip;
			unsigned short port;
			bool isKeepalived;
		} Server;

		std::string title;
		Server server;
		double PI;
	} Config;

	static const Config readConfig()
	{
		inicpp::IniManager _ini(CONFIG_FILE);

		return Config{
			_ini[""]["title"],
			Config::Server{
				_ini["server"]["ip"],
				_ini["server"]["port"],
				_ini["server"]["isKeepalived"]},
			_ini["math"]["PI"],
		};
	}
};

int main()
{
	/** easy read for app as struct */
	app::Config config = app::readConfig();

	std::cout << config.server.ip << std::endl;

	return 0;
}
```


#### 8.how to use example/main.cpp
You can compile it using `example/Makefile` or any other method you prefer.

If make is not available, use the following command: `g++ -I../ -std=c++11 main.cpp -o iniExample`.

- Compile `example/main.cpp`
```bash
jn@jn:~/inicpp/example$ ls
example  inicpp.hpp  LICENSE  README.md
jn@jn:~/inicpp/example$ cd example/
jn@jn:~/inicpp/example$ make
g++ -I../ -std=c++11 main.cpp -o iniExample
jn@jn:~/inicpp/example$ ls
iniExample  main.cpp  Makefile
```

- Run example app `iniExample`
```bash
jn@jn:~/inicpp/example$ ./iniExample

Section:
title      -------> config.ini

Section: math
PI         -------> 3.141592653589793238462643383279502884

Section: server
info       -------> the server socket info.
ip         -------> 127.0.0.1
keepalived -------> true
number     -------> 1
port       -------> 8080
jn@jn:~/inicpp/example$
```

- Configuration file `config.ini` has been created.
```bash
jn@jn:~/inicpp/example$ ls
config.ini  iniExample  main.cpp  Makefile
jn@jn:~/inicpp/example$ cat config.ini
;This is the title.
title=config.ini
[server]
number=1
info=the server socket info.
keepalived=true
;this is the listen ip for server.
port=8080
ip=127.0.0.1


[math]
;Comment: This is pi in mathematics.
PI=3.141592653589793238462643383279502884
jn@jn:~/inicpp/example$
```

---

### Ⅳ、Star History
<a href="https://star-history.com/#dujingning/inicpp">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=dujingning/inicpp&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=dujingning/inicpp&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=dujingning/inicpp&type=Date" />
 </picture>
</a>

---


### Ⅴ、End
 The project was created by **DuJingning**.
