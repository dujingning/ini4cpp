CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -pedantic -I..

TESTS := inicpp_regression

.PHONY: all test clean

all: test

inicpp_regression: inicpp_regression.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_regression.cpp -o inicpp_regression

test: $(TESTS)
	./inicpp_regression

clean:
	rm -f $(TESTS)
