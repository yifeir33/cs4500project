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
CXXFLAGS     := -Wall -Wextra -Wpedantic -g -pthread -std=c++17 -I$(INCLUDE) -I$(BOAT)/include/

.PHONY: all server client test clean directories valgrind

# Makes main binary (which isn't written right now)
all: server client

server: directories $(BIN)/server

client: directories $(BIN)/client

# Makes and executes test binary
test: directories $(BIN)/tests
	$(BIN)/tests --success

valgrind: directories $(BIN)/tests
	valgrind --leak-check=full $(BIN)/tests

directories: $(OBJ) $(BIN)

$(OBJ):
	mkdir -p $@

$(BIN):
	mkdir -p $@

# server binary
$(BIN)/server: $(SRC_OBJS) $(BOAT)/lib/libsorer.a $(OBJ)/server_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# client binary
$(BIN)/client: $(SRC_OBJS) $(BOAT)/lib/libsorer.a $(OBJ)/client_main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Test binary
$(BIN)/tests: $(SRC_OBJS) $(TEST_OBJS) $(BOAT)/lib/libsorer.a
	$(CXX) $(CXXFLAGS) $^ -o $@

# server object
$(OBJ)/server_main.o: $(SRC)/server_main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# client object
$(OBJ)/client_main.o: $(SRC)/client_main.cpp
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
	rm $(OBJ)/*.o $(BIN)/tests; $(MAKE) -C $(BOAT) clean
