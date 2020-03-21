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
STORE        := $(SRC)/store
ADAPTER      := $(SRC)/adapter

TEST_SRCS    := $(wildcard $(TESTS)/*.cpp)
UTIL_SRCS    := $(wildcard $(UTIL)/*.cpp)
NETWORK_SRCS := $(wildcard $(NETWORK)/*.cpp)
DATA_SRCS    := $(wildcard $(DATA)/*.cpp)
STORE_SRCS   := $(wildcard $(STORE)/*.cpp)
ADAPTER_SRCS := $(wildcard $(ADAPTER)/*.cpp)

TEST_OBJS    := $(patsubst $(TESTS)/%.cpp, $(OBJ)/%.o, $(TEST_SRCS))
UTIL_OBJS    := $(patsubst $(UTIL)/%.cpp, $(OBJ)/%.o, $(UTIL_SRCS))
NETWORK_OBJS := $(patsubst $(NETWORK)/%.cpp, $(OBJ)/%.o, $(NETWORK_SRCS))
DATA_OBJS    := $(patsubst $(DATA)/%.cpp, $(OBJ)/%.o, $(DATA_SRCS))
STORE_OBJS   := $(patsubst $(STORE)/%.cpp, $(OBJ)/%.o, $(STORE_SRCS))
ADAPTER_OBJS := $(patsubst $(ADAPTER)/%.cpp, $(OBJ)/%.o, $(ADAPTER_SRCS))

SRC_OBJS     := $(UTIL_OBJS) $(NETWORK_OBJS) $(DATA_OBJS) $(STORE_OBJS) $(ADAPTER_OBJS)

CXX          := g++
CXXFLAGS     := -Wall -Wextra -Wpedantic -g -pthread -std=c++17 -I$(INCLUDE) -I$(BOAT)/include/

.PHONY: all test clean

# Makes main binary (which isn't written right now)
all: $(SRC_OBJS) $(BOAT)/lib/libsorer.a
	echo "All Objects Built ;)"

# Makes and executes test binary
test: $(BIN)/tests
	$< --success

# Test binary
$(BIN)/tests: $(SRC_OBJS) $(TEST_OBJS) $(BOAT)/lib/libsorer.a
	$(CXX) $(CXXFLAGS) -L$(BOAT)/lib/ -lsorer $^ -o $@

# Util
$(OBJ)/%.o: $(UTIL)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Network
$(OBJ)/%.o: $(NETWORK)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Data
$(OBJ)/%.o: $(DATA)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Store
$(OBJ)/%.o: $(STORE)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Adapter
$(OBJ)/%.o: $(ADAPTER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Sorer library - This is a submodule
$(BOAT)/lib/libsorer.a: $(BOAT)/Makefile
	cd $(BOAT) && $(MAKE) 

# Tests
$(OBJ)/%.o: $(TESTS)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(OBJ)/*.o $(BIN)/tests
	cd $(BOAT) && make clean
