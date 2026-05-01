#define _ENABLE_INICPP_STD_WSTRING_
#include "../inicpp.hpp"

#include <cstdio>
#include <iostream>
#include <string>

int main()
{
	const char *path = "inicpp_wstring_compat.ini";
	std::remove(path);

	inicpp::IniManager ini(L"inicpp_wstring_compat.ini");
	ini.setFileName(L"inicpp_wstring_compat.ini");
	if (!ini.set("server", "info", std::wstring(L"hello")))
	{
		return 1;
	}

	std::remove(path);
	std::cout << "inicpp_wstring_compat: PASS" << std::endl;
	return 0;
}
