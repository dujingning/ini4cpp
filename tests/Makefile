CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -pedantic -I..

TESTS := inicpp_regression inicpp_wstring_legacy inicpp_wstring_compat inicpp_file_safety inicpp_ini_format

.PHONY: all test clean

all: test

inicpp_regression: inicpp_regression.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_regression.cpp -o inicpp_regression

inicpp_wstring_legacy: inicpp_wstring_legacy.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_wstring_legacy.cpp -o inicpp_wstring_legacy

inicpp_wstring_compat: inicpp_wstring_compat.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_wstring_compat.cpp -o inicpp_wstring_compat

inicpp_file_safety: inicpp_file_safety.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_file_safety.cpp -o inicpp_file_safety

inicpp_ini_format: inicpp_ini_format.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_ini_format.cpp -o inicpp_ini_format

test: $(TESTS)
	./inicpp_regression
	./inicpp_wstring_legacy
	./inicpp_wstring_compat
	./inicpp_file_safety
	./inicpp_ini_format

clean:
	rm -f $(TESTS)
