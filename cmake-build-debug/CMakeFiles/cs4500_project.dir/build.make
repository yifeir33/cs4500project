# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/wangyifei/cs4500_project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/wangyifei/cs4500_project/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/cs4500_project.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/cs4500_project.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cs4500_project.dir/flags.make

CMakeFiles/cs4500_project.dir/main.cpp.o: CMakeFiles/cs4500_project.dir/flags.make
CMakeFiles/cs4500_project.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/wangyifei/cs4500_project/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cs4500_project.dir/main.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/cs4500_project.dir/main.cpp.o -c /Users/wangyifei/cs4500_project/main.cpp

CMakeFiles/cs4500_project.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cs4500_project.dir/main.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/wangyifei/cs4500_project/main.cpp > CMakeFiles/cs4500_project.dir/main.cpp.i

CMakeFiles/cs4500_project.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cs4500_project.dir/main.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/wangyifei/cs4500_project/main.cpp -o CMakeFiles/cs4500_project.dir/main.cpp.s

# Object files for target cs4500_project
cs4500_project_OBJECTS = \
"CMakeFiles/cs4500_project.dir/main.cpp.o"

# External object files for target cs4500_project
cs4500_project_EXTERNAL_OBJECTS =

cs4500_project: CMakeFiles/cs4500_project.dir/main.cpp.o
cs4500_project: CMakeFiles/cs4500_project.dir/build.make
cs4500_project: CMakeFiles/cs4500_project.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/wangyifei/cs4500_project/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable cs4500_project"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cs4500_project.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cs4500_project.dir/build: cs4500_project

.PHONY : CMakeFiles/cs4500_project.dir/build

CMakeFiles/cs4500_project.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cs4500_project.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cs4500_project.dir/clean

CMakeFiles/cs4500_project.dir/depend:
	cd /Users/wangyifei/cs4500_project/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/wangyifei/cs4500_project /Users/wangyifei/cs4500_project /Users/wangyifei/cs4500_project/cmake-build-debug /Users/wangyifei/cs4500_project/cmake-build-debug /Users/wangyifei/cs4500_project/cmake-build-debug/CMakeFiles/cs4500_project.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cs4500_project.dir/depend
