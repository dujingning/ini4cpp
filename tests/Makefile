CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -pedantic -I..

TESTS := inicpp_regression inicpp_wstring_legacy inicpp_wstring_compat

.PHONY: all test clean

all: test

inicpp_regression: inicpp_regression.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_regression.cpp -o inicpp_regression

inicpp_wstring_legacy: inicpp_wstring_legacy.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_wstring_legacy.cpp -o inicpp_wstring_legacy

inicpp_wstring_compat: inicpp_wstring_compat.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_wstring_compat.cpp -o inicpp_wstring_compat

test: $(TESTS)
	./inicpp_regression
	./inicpp_wstring_legacy
	./inicpp_wstring_compat

clean:
	rm -f $(TESTS)
