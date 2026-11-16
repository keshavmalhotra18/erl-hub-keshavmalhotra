CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS += -Iinclude

LIB_SRC  := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
LIB_OBJ  := $(LIB_SRC:src/%.cpp=build/%.o)

.PHONY: all run test clean
all: build/erl_hub build/erl_tests

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/main.o: src/main.cpp | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/test_main.o: tests/test_main.cpp | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/erl_hub: build/main.o $(LIB_OBJ)
	$(CXX) $^ -o $@

build/erl_tests: build/test_main.o $(LIB_OBJ)
	$(CXX) $^ -o $@

build:
	mkdir -p build

run: build/erl_hub
	./build/erl_hub

test: build/erl_tests
	./build/erl_tests

clean:
	rm -rf build *.bin
