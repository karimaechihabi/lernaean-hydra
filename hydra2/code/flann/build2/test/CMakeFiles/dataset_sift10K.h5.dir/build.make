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

# Utility rule file for dataset_sift10K.h5.

# Include the progress variables for this target.
include test/CMakeFiles/dataset_sift10K.h5.dir/progress.make

test/CMakeFiles/dataset_sift10K.h5:
	cd /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test && /opt/anaconda2/bin/python /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/bin/download_checkmd5.py "http://people.cs.ubc.ca/~mariusm/uploads/FLANN/datasets/sift10K.h5" /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/test/sift10K.h5 0964a910946d2dd5fe28337507a8abc3

dataset_sift10K.h5: test/CMakeFiles/dataset_sift10K.h5
dataset_sift10K.h5: test/CMakeFiles/dataset_sift10K.h5.dir/build.make

.PHONY : dataset_sift10K.h5

# Rule to build all files generated by this target.
test/CMakeFiles/dataset_sift10K.h5.dir/build: dataset_sift10K.h5

.PHONY : test/CMakeFiles/dataset_sift10K.h5.dir/build

test/CMakeFiles/dataset_sift10K.h5.dir/clean:
	cd /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test && $(CMAKE_COMMAND) -P CMakeFiles/dataset_sift10K.h5.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/dataset_sift10K.h5.dir/clean

test/CMakeFiles/dataset_sift10K.h5.dir/depend:
	cd /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/test /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2 /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test /home/karimae/projects/dsseval_full_compact_journal/code/flann-proj/build2/test/CMakeFiles/dataset_sift10K.h5.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/dataset_sift10K.h5.dir/depend

