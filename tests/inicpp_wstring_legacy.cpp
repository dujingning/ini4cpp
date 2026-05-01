#define _ENBABLE_INICPP_STD_WSTRING_
#include "../inicpp.hpp"

#include <cstdio>
#include <iostream>
#include <string>

int main()
{
	const char *path = "inicpp_wstring_legacy.ini";
	std::remove(path);

	inicpp::IniManager ini(L"inicpp_wstring_legacy.ini");
	if (!ini.set("server", "info", std::wstring(L"hello")))
	{
		return 1;
	}

	inicpp::IniManager loaded(L"inicpp_wstring_legacy.ini");
	if (loaded["server"].toWString("info") != L"hello")
	{
		return 2;
	}

	std::remove(path);
	std::cout << "inicpp_wstring_legacy: PASS" << std::endl;
	return 0;
}
