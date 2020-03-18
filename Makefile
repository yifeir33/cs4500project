MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
TOP_DIR     := $(dir $(MKFILE_PATH))

SRC         := $(TOP_DIR)src
TESTS       := $(TOP_DIR)tests
OBJ         := $(TOP_DIR)objs
BIN         := $(TOP_DIR)bin

SOURCES     := $(wildcard $(SRC)/*.cpp)
OBJECTS     := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))
TEST_SRCS   := $(wildcard $(TESTS)/*.cpp)
TEST_OBJS   := $(patsubst $(TESTS)/%.cpp, $(OBJ)/%.o, $(TEST_SRCS))

CXX         := g++
CXXFLAGS    := -Wall -Wextra -Wpedantic -g -pthread -std=c++17 -I$(SRC)

.PHONY: all test clean

all: $(OBJECTS)
	echo "All Objects Built ;)"

test: $(BIN)/tests
	$<

$(BIN)/tests: $(OBJECTS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(TESTS)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(OBJ)/*.o $(BIN)/tests
