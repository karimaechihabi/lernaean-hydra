# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2

# Utility rule file for test.

# Include the progress variables for this target.
include test/CMakeFiles/test.dir/progress.make

test: test/CMakeFiles/test.dir/build.make

.PHONY : test

# Rule to build all files generated by this target.
test/CMakeFiles/test.dir/build: test

.PHONY : test/CMakeFiles/test.dir/build

test/CMakeFiles/test.dir/clean:
	cd /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test && $(CMAKE_COMMAND) -P CMakeFiles/test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test.dir/clean

test/CMakeFiles/test.dir/depend:
	cd /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/test /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2 /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test/CMakeFiles/test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test.dir/depend

