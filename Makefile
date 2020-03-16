# I wish I was a makefile wizard
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
TOP_DIR     := $(dir $(MKFILE_PATH))

SRC := $(TOP_DIR)src
OBJ := $(TOP_DIR)objs
BIN := $(TOP_DIR)bin

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

CXX=clang++
CXXFLAGS=-Wall -Wextra -Wpedantic -g -pthread -std=c++17

.PHONY: all clean

all: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC) -c $< -o $@

clean:
	rm $(OBJ)/*.o
