MKFILE_PATH  := $(abspath $(lastword $(MAKEFILE_LIST)))
TOP_DIR      := $(dir $(MKFILE_PATH))

INCLUDE      := $(TOP_DIR)include
SRC          := $(TOP_DIR)src
TESTS        := $(TOP_DIR)tests
OBJ          := $(TOP_DIR)objs
BIN          := $(TOP_DIR)bin
BOAT         := $(TOP_DIR)boat-a1p1

UTIL         := $(SRC)/util
NETWORK      := $(SRC)/network
DATA         := $(SRC)/data
ADAPTER      := $(SRC)/adapter

TEST_SRCS    := $(wildcard $(TESTS)/*.cpp)
UTIL_SRCS    := $(wildcard $(UTIL)/*.cpp)
NETWORK_SRCS := $(wildcard $(NETWORK)/*.cpp)
DATA_SRCS    := $(wildcard $(DATA)/*.cpp)
ADAPTER_SRCS := $(wildcard $(ADAPTER)/*.cpp)

TEST_OBJS    := $(patsubst $(TESTS)/%.cpp, $(OBJ)/%.o, $(TEST_SRCS))
UTIL_OBJS    := $(patsubst $(UTIL)/%.cpp, $(OBJ)/%.o, $(UTIL_SRCS))
NETWORK_OBJS := $(patsubst $(NETWORK)/%.cpp, $(OBJ)/%.o, $(NETWORK_SRCS))
DATA_OBJS    := $(patsubst $(DATA)/%.cpp, $(OBJ)/%.o, $(DATA_SRCS))
ADAPTER_OBJS := $(patsubst $(ADAPTER)/%.cpp, $(OBJ)/%.o, $(ADAPTER_SRCS))

SRC_OBJS     := $(UTIL_OBJS) $(NETWORK_OBJS) $(DATA_OBJS) $(ADAPTER_OBJS)

CXX          := g++
CXXFLAGS     := -Wall -Wextra -Wpedantic -g -O3 -pthread -std=c++17 -I$(INCLUDE) -I$(BOAT)/include/

.PHONY: all demo test clean directories valgrind

all: directories $(BIN)/demo

# have fun killing these background proccess ;)
demo: directories $(BIN)/demo
	$(BIN)/demo --server > /dev/null 2>&1 & 
	$(BIN)/demo --producer > /dev/null 2>&1 &
	$(BIN)/demo --counter > /dev/null 2>&1 &
	$(BIN)/demo --summarizer

# Makes and executes test binary
test: directories $(BIN)/tests
	$(BIN)/tests

valgrind: directories $(BIN)/demo $(BIN)/tests
	valgrind --leak-check=full -s $(BIN)/tests
	valgrind --leak-check=full -s $(BIN)/demo --server &
	valgrind --leak-check=full -s $(BIN)/demo --producer &
	valgrind --leak-check=full -s $(BIN)/demo --counter &
	valgrind --leak-check=full -s $(BIN)/demo --summarizer &

directories: $(OBJ) $(BIN)

$(OBJ):
	mkdir -p $@

$(BIN):
	mkdir -p $@

$(BIN)/demo: $(SRC_OBJS) $(BOAT)/lib/libsorer.a $(OBJ)/demo_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Test binary
$(BIN)/tests: $(SRC_OBJS) $(TEST_OBJS) $(BOAT)/lib/libsorer.a
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ)/demo_main.o: $(SRC)/demo_main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Util
$(OBJ)/%.o: $(UTIL)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Network
$(OBJ)/%.o: $(NETWORK)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Data
$(OBJ)/%.o: $(DATA)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Adapter
$(OBJ)/%.o: $(ADAPTER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Sorer library - This is a submodule
$(BOAT)/lib/libsorer.a: $(BOAT)/Makefile FORCE
	$(MAKE) -C $(BOAT)

# This forces it to always try to build the submodule
FORCE: ;

# Tests
$(OBJ)/%.o: $(TESTS)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(OBJ)/*.o $(BIN)/tests $(BIN)/demo; $(MAKE) -C $(BOAT) clean
