CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -pedantic -I..
EXEEXT ?=

BINARY_TESTS := \
	inicpp_regression \
	inicpp_wstring_legacy \
	inicpp_wstring_compat \
	inicpp_file_safety \
	inicpp_ini_format \
	inicpp_value_proxy \
	inicpp_deferred_write \
	inicpp_write_paths \
	inicpp_write_failures \
	inicpp_section_ini_api \
	inicpp_parse_edges \
	inicpp_header_integration

.PHONY: all test clean example_compile negative_compile_tests negative_value_proxy_value_constructor

all: test

TEST_BINS := $(addsuffix $(EXEEXT),$(BINARY_TESTS))

example_compile: ../example/main.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) -pedantic-errors -c ../example/main.cpp -o example_main.o
	@echo "example_compile: PASS"

inicpp_regression$(EXEEXT): inicpp_regression.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_regression.cpp -o $@

inicpp_wstring_legacy$(EXEEXT): inicpp_wstring_legacy.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_wstring_legacy.cpp -o $@

inicpp_wstring_compat$(EXEEXT): inicpp_wstring_compat.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_wstring_compat.cpp -o $@

inicpp_file_safety$(EXEEXT): inicpp_file_safety.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_file_safety.cpp -o $@

inicpp_ini_format$(EXEEXT): inicpp_ini_format.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_ini_format.cpp -o $@

inicpp_value_proxy$(EXEEXT): inicpp_value_proxy.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_value_proxy.cpp -o $@

inicpp_deferred_write$(EXEEXT): inicpp_deferred_write.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_deferred_write.cpp -o $@

inicpp_write_paths$(EXEEXT): inicpp_write_paths.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_write_paths.cpp -o $@

inicpp_write_failures$(EXEEXT): inicpp_write_failures.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_write_failures.cpp -o $@

inicpp_section_ini_api$(EXEEXT): inicpp_section_ini_api.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_section_ini_api.cpp -o $@

inicpp_parse_edges$(EXEEXT): inicpp_parse_edges.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_parse_edges.cpp -o $@

inicpp_header_integration$(EXEEXT): inicpp_header_integration.cpp ../inicpp.hpp
	$(CXX) $(CXXFLAGS) inicpp_header_integration.cpp -o $@

negative_value_proxy_value_constructor: negative/value_proxy_value_constructor.cpp ../inicpp.hpp
	@rm -f negative_value_proxy_value_constructor.o negative_value_proxy_value_constructor.log
	@if $(CXX) $(CXXFLAGS) -c negative/value_proxy_value_constructor.cpp -o negative_value_proxy_value_constructor.o > negative_value_proxy_value_constructor.log 2>&1; then \
		cat negative_value_proxy_value_constructor.log; \
		echo "negative_value_proxy_value_constructor: expected compilation to fail"; \
		rm -f negative_value_proxy_value_constructor.o negative_value_proxy_value_constructor.log; \
		exit 1; \
	else \
		rm -f negative_value_proxy_value_constructor.o negative_value_proxy_value_constructor.log; \
		echo "negative_value_proxy_value_constructor: PASS"; \
	fi

negative_compile_tests: negative_value_proxy_value_constructor

test: example_compile $(TEST_BINS)
	./inicpp_regression$(EXEEXT)
	./inicpp_wstring_legacy$(EXEEXT)
	./inicpp_wstring_compat$(EXEEXT)
	./inicpp_file_safety$(EXEEXT)
	./inicpp_ini_format$(EXEEXT)
	./inicpp_value_proxy$(EXEEXT)
	./inicpp_deferred_write$(EXEEXT)
	./inicpp_write_paths$(EXEEXT)
	./inicpp_write_failures$(EXEEXT)
	./inicpp_section_ini_api$(EXEEXT)
	./inicpp_parse_edges$(EXEEXT)
	./inicpp_header_integration$(EXEEXT)
	$(MAKE) --no-print-directory negative_compile_tests

clean:
	rm -f $(TEST_BINS) example_main.o negative_value_proxy_value_constructor.o negative_value_proxy_value_constructor.log
