# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /opt/local/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jimbolaptop/pNXT

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jimbolaptop/pNXT/src/gui

# Include any dependencies generated for this target.
include tests/CMakeFiles/net_load_tests_clt.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/net_load_tests_clt.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/net_load_tests_clt.dir/flags.make

tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o: tests/CMakeFiles/net_load_tests_clt.dir/flags.make
tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o: ../../tests/net_load_tests/clt.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/jimbolaptop/pNXT/src/gui/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o"
	cd /Users/jimbolaptop/pNXT/src/gui/tests && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o -c /Users/jimbolaptop/pNXT/tests/net_load_tests/clt.cpp

tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.i"
	cd /Users/jimbolaptop/pNXT/src/gui/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /Users/jimbolaptop/pNXT/tests/net_load_tests/clt.cpp > CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.i

tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.s"
	cd /Users/jimbolaptop/pNXT/src/gui/tests && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /Users/jimbolaptop/pNXT/tests/net_load_tests/clt.cpp -o CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.s

tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.requires:
.PHONY : tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.requires

tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.provides: tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.requires
	$(MAKE) -f tests/CMakeFiles/net_load_tests_clt.dir/build.make tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.provides.build
.PHONY : tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.provides

tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.provides.build: tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o

# Object files for target net_load_tests_clt
net_load_tests_clt_OBJECTS = \
"CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o"

# External object files for target net_load_tests_clt
net_load_tests_clt_EXTERNAL_OBJECTS =

tests/net_load_tests_clt: tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o
tests/net_load_tests_clt: tests/CMakeFiles/net_load_tests_clt.dir/build.make
tests/net_load_tests_clt: src/libcurrency_core.a
tests/net_load_tests_clt: src/libcommon.a
tests/net_load_tests_clt: src/libcrypto.a
tests/net_load_tests_clt: tests/gtest/libgtest_main.a
tests/net_load_tests_clt: /opt/local/lib/libboost_system-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_filesystem-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_thread-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_date_time-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_chrono-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_regex-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_serialization-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_atomic-mt.a
tests/net_load_tests_clt: /opt/local/lib/libboost_program_options-mt.a
tests/net_load_tests_clt: tests/gtest/libgtest.a
tests/net_load_tests_clt: tests/CMakeFiles/net_load_tests_clt.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable net_load_tests_clt"
	cd /Users/jimbolaptop/pNXT/src/gui/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/net_load_tests_clt.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/net_load_tests_clt.dir/build: tests/net_load_tests_clt
.PHONY : tests/CMakeFiles/net_load_tests_clt.dir/build

tests/CMakeFiles/net_load_tests_clt.dir/requires: tests/CMakeFiles/net_load_tests_clt.dir/net_load_tests/clt.cpp.o.requires
.PHONY : tests/CMakeFiles/net_load_tests_clt.dir/requires

tests/CMakeFiles/net_load_tests_clt.dir/clean:
	cd /Users/jimbolaptop/pNXT/src/gui/tests && $(CMAKE_COMMAND) -P CMakeFiles/net_load_tests_clt.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/net_load_tests_clt.dir/clean

tests/CMakeFiles/net_load_tests_clt.dir/depend:
	cd /Users/jimbolaptop/pNXT/src/gui && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jimbolaptop/pNXT /Users/jimbolaptop/pNXT/tests /Users/jimbolaptop/pNXT/src/gui /Users/jimbolaptop/pNXT/src/gui/tests /Users/jimbolaptop/pNXT/src/gui/tests/CMakeFiles/net_load_tests_clt.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/net_load_tests_clt.dir/depend

