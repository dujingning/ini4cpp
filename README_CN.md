### 这个 C++ 库用于什么？

> 1. 让 INI 文件处理保持**简单**、**方便**、**直观**。

> 2. 通过协作创新推进 **C++** 的长期价值。

> 3. 让 C++ 保持**简单**、**易用**、**自由**。

---

### Ⅰ、项目

项目地址：

- GitHub: [https://github.com/dujingning/inicpp.git](https://github.com/dujingning/inicpp.git)
- Gitee: [https://gitee.com/dujingning/inicpp](https://gitee.com/dujingning/inicpp)

#### * 如果这个项目对你有帮助，欢迎点一个 star。遇到问题也欢迎提交 issue。

---

### Ⅱ、说明

inicpp 是一个面向 Modern C++ 的 INI 单头文件库，支持**读取**、**写入**和**注释写入**。它使用简单，可以降低 C++ 项目处理 INI 配置文件的成本。

- 新特性：[将配置轻松绑定到业务数据结构（读取）](#7将配置轻松绑定到业务数据结构读取)。

#### 支持的 INI 语法

读取 INI 文件时，inicpp 支持以下常见形式：

- 文件开头的 UTF-8 BOM。
- section 头，例如 `[server]`、`  [server]`、`[ server ]`。
- 使用 `=` 或 `:` 的 key-value 行，例如 `port=8080` 和 `host: localhost`。
- 文件中已有的空值，以及通过 `set(..., "")` 写入的空值，例如 `key=`。
- 使用 `;` 或 `#` 的整行注释，包括前面带空白字符的注释行。
- 位于空白字符之后的行内注释，例如 `key=value ; comment` 和 `key=value # comment`。
- 位于引号或转义值中的注释标记会被保留，例如 `text="a ; b"` 和 `path=C:\tmp;cache`。
- 同一个 section 中的重复 key，以及重复 section 头，都会使用最后一次解析到的重复 key 值。

带引号的值会按原样返回；inicpp 不会自动去除引号，也不会自动反转义。写入操作会保留已有文件的 LF 或 CRLF 换行风格，但不支持多行值、不带 `=` 或 `:` 的裸 key，也不保证完整保留原始格式进行 round trip。

---

### Ⅲ、用法

#### * 0. 使用 C++11 或更高版本。

```bash
git clone https://github.com/dujingning/inicpp.git
```

包含 `inicpp.hpp`，声明 `inicpp::IniManager` 对象即可使用。

#### 1. 写入示例

写入：直接写入文件。

```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // 加载并解析 INI 文件。

    _ini["server"]["ip"] = "192.168.3.35";
    _ini["server"]["port"] = 554;
    std::cout << _ini["server"]["ip"] << ":"<< _ini["server"]["port"] << std::endl;

    // 或使用 set 写入
    _ini.set("server","ip","127.0.0.1");
    _ini.set("server","port",8080);
    std::cout << _ini["server"]["ip"] << ":"<< _ini["server"]["port"] << std::endl;
}
```

#### 1.1 延迟写入示例

默认情况下，`set()` 和 `operator[]` 赋值会立即写入文件。如果需要一次更新多个配置项，可以关闭自动 flush，先把修改保存在内存中，最后调用一次 `flush()` 写入文件。

```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // 加载并解析 INI 文件。

    if (!_ini.setAutoFlush(false))
    {
        return 1;
    }

    _ini.set("server", "ip", "127.0.0.1"); // 只修改内存。
    _ini.set("server", "port", 8080);       // 只修改内存。
    _ini["server"]["name"] = "main";        // 只修改内存。

    if (_ini.isDirty() && !_ini.flush())
    {
        std::cerr << "failed to write config.ini" << std::endl;
        return 1;
    }
}
```

`flush()` 会通过与立即写入相同的 backup 替换流程，把所有 pending 修改写入磁盘。析构函数不会自动调用 `flush()`，这样调用者可以显式处理写入失败。调用 `parse()` 会丢弃尚未 `flush()` 的修改，并重新加载文件。存在 pending 修改时调用 `setAutoFlush(true)` 会先尝试 `flush()`；如果写入失败，则返回 `false`。

#### 2. 读取示例

转换：将字符串转换为目标类型。`operator[]` 的值转换，包括 `std::string`、`bool` 和 `get<T>()`，在 section/key 不存在或类型转换失败时会抛出 `std::runtime_error`。文件中已有的空值是合法值，转换为 `std::string` 时会得到空字符串；如果需要不抛异常的默认值读取，可以使用 `toString()`、`toInt()` 和 `toDouble()`。`bool` 转换只会把 `"0"`、`"false"` 和 `"no"` 视为 false。

```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // 加载并解析 INI 文件。

    int         port = _ini["server"]["port"];
    std::string ip   = _ini["server"]["ip"];
    std::cout << ip << ":"<< port << std::endl;

    // 或使用 get 获取
    ip   = _ini["server"]["ip"].get<std::string>();
    port = _ini["server"]["port"].get<int>();
    std::cout << ip << ":"<< port << std::endl;
}
```

#### 3. 注释示例

注释：为 key-value 写入注释。

```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // 加载并解析 INI 文件。

    // section/key 注释
    _ini.set("math"/*section*/, "PI"/*key*/, "3.1415926535897932"/*key*/, "This is PI in mathematics."/*comment*/);
    _ini.setComment("server"/*section*/, "port"/*key*/, "this is the listen ip for server."/*comment*/);
}
```

通过 `set()` 或 `setComment()` 写入的注释默认会添加 `;` 前缀。如果传入的注释字符串已经以 `;` 或 `#` 开头，则会保留原有标记。

#### 4. `toString()`、`toInt()`、`toDouble()`

转换：从字符串转换为目标类型，但不抛异常。key 不存在时，`toString()` 返回空字符串，`toInt()` 和 `toDouble()` 分别返回 `0` 或 `0.0`；转换失败时，`toInt()` 和 `toDouble()` 也返回默认值。

```cpp
#include "inicpp.hpp"
#include <iostream>

int main()
{
    inicpp::IniManager _ini("config.ini"); // 加载并解析 INI 文件。

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

#### 5. `isKeyExists()`、`sectionsList()`、`sectionMap()`

支持无名 section：例如 key 位于文件头部时。

```cpp
#include "inicpp.hpp"
#include <iostream>
#include <iomanip>

// 绿色文本的 ANSI 转义码
#define GREEN_TEXT "\033[1;32m"
#define RESET_COLOR "\033[0m"

int main()
{
    inicpp::IniManager _ini("config.ini"); // 加载并解析 INI 文件。

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

#### 6. `std::wstring`

```cpp
#ifndef _ENBABLE_INICPP_STD_WSTRING_
#define _ENBABLE_INICPP_STD_WSTRING_ // std::wstring support
#endif

#include "inicpp.hpp"

#include <bits/stdc++.h>

int main()
{
    {
        inicpp::IniManager _ini(L"config.ini"); // 加载并解析 INI 文件。
        std::wstring ws = _ini["server"].toWString("info");
        int port = _ini["server"]["port"];
        std::string ip = _ini["server"]["ip"];

        std::cout << ip << ":" << port << std::endl;
    }
    // 或者
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

#### 7. 将配置轻松绑定到业务数据结构（读取）

config.ini:

```ini
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

config.cpp:

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

#### 8. 如何使用 `example/main.cpp`

可以通过 `example/Makefile` 编译，也可以使用你习惯的其他方式。

如果无法使用 make，可以执行：

```bash
g++ -I../ -std=c++11 main.cpp -o iniExample
```

- 编译 `example/main.cpp`

```bash
jn@jn:~/inicpp/example$ ls
example  inicpp.hpp  LICENSE  README.md
jn@jn:~/inicpp/example$ cd example/
jn@jn:~/inicpp/example$ make
g++ -I../ -std=c++11 main.cpp -o iniExample
jn@jn:~/inicpp/example$ ls
iniExample  main.cpp  Makefile
```

- 运行示例程序 `iniExample`

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

- 生成的配置文件 `config.ini`

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

### Ⅴ、结束

本项目由 **DuJingning** 创建。
