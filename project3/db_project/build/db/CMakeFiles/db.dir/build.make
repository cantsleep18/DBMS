# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cantsleep/2021_ite2038_2018007429/project3/db_project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build

# Include any dependencies generated for this target.
include db/CMakeFiles/db.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include db/CMakeFiles/db.dir/compiler_depend.make

# Include the progress variables for this target.
include db/CMakeFiles/db.dir/progress.make

# Include the compile flags for this target's objects.
include db/CMakeFiles/db.dir/flags.make

db/CMakeFiles/db.dir/src/bpt.cc.o: db/CMakeFiles/db.dir/flags.make
db/CMakeFiles/db.dir/src/bpt.cc.o: ../db/src/bpt.cc
db/CMakeFiles/db.dir/src/bpt.cc.o: db/CMakeFiles/db.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object db/CMakeFiles/db.dir/src/bpt.cc.o"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT db/CMakeFiles/db.dir/src/bpt.cc.o -MF CMakeFiles/db.dir/src/bpt.cc.o.d -o CMakeFiles/db.dir/src/bpt.cc.o -c /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/bpt.cc

db/CMakeFiles/db.dir/src/bpt.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db.dir/src/bpt.cc.i"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/bpt.cc > CMakeFiles/db.dir/src/bpt.cc.i

db/CMakeFiles/db.dir/src/bpt.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db.dir/src/bpt.cc.s"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/bpt.cc -o CMakeFiles/db.dir/src/bpt.cc.s

db/CMakeFiles/db.dir/src/file.cc.o: db/CMakeFiles/db.dir/flags.make
db/CMakeFiles/db.dir/src/file.cc.o: ../db/src/file.cc
db/CMakeFiles/db.dir/src/file.cc.o: db/CMakeFiles/db.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object db/CMakeFiles/db.dir/src/file.cc.o"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT db/CMakeFiles/db.dir/src/file.cc.o -MF CMakeFiles/db.dir/src/file.cc.o.d -o CMakeFiles/db.dir/src/file.cc.o -c /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/file.cc

db/CMakeFiles/db.dir/src/file.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db.dir/src/file.cc.i"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/file.cc > CMakeFiles/db.dir/src/file.cc.i

db/CMakeFiles/db.dir/src/file.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db.dir/src/file.cc.s"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/file.cc -o CMakeFiles/db.dir/src/file.cc.s

db/CMakeFiles/db.dir/src/buffer.cc.o: db/CMakeFiles/db.dir/flags.make
db/CMakeFiles/db.dir/src/buffer.cc.o: ../db/src/buffer.cc
db/CMakeFiles/db.dir/src/buffer.cc.o: db/CMakeFiles/db.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object db/CMakeFiles/db.dir/src/buffer.cc.o"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT db/CMakeFiles/db.dir/src/buffer.cc.o -MF CMakeFiles/db.dir/src/buffer.cc.o.d -o CMakeFiles/db.dir/src/buffer.cc.o -c /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/buffer.cc

db/CMakeFiles/db.dir/src/buffer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db.dir/src/buffer.cc.i"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/buffer.cc > CMakeFiles/db.dir/src/buffer.cc.i

db/CMakeFiles/db.dir/src/buffer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db.dir/src/buffer.cc.s"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db/src/buffer.cc -o CMakeFiles/db.dir/src/buffer.cc.s

# Object files for target db
db_OBJECTS = \
"CMakeFiles/db.dir/src/bpt.cc.o" \
"CMakeFiles/db.dir/src/file.cc.o" \
"CMakeFiles/db.dir/src/buffer.cc.o"

# External object files for target db
db_EXTERNAL_OBJECTS =

lib/libdb.a: db/CMakeFiles/db.dir/src/bpt.cc.o
lib/libdb.a: db/CMakeFiles/db.dir/src/file.cc.o
lib/libdb.a: db/CMakeFiles/db.dir/src/buffer.cc.o
lib/libdb.a: db/CMakeFiles/db.dir/build.make
lib/libdb.a: db/CMakeFiles/db.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library ../lib/libdb.a"
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && $(CMAKE_COMMAND) -P CMakeFiles/db.dir/cmake_clean_target.cmake
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/db.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
db/CMakeFiles/db.dir/build: lib/libdb.a
.PHONY : db/CMakeFiles/db.dir/build

db/CMakeFiles/db.dir/clean:
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db && $(CMAKE_COMMAND) -P CMakeFiles/db.dir/cmake_clean.cmake
.PHONY : db/CMakeFiles/db.dir/clean

db/CMakeFiles/db.dir/depend:
	cd /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cantsleep/2021_ite2038_2018007429/project3/db_project /home/cantsleep/2021_ite2038_2018007429/project3/db_project/db /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db /home/cantsleep/2021_ite2038_2018007429/project3/db_project/build/db/CMakeFiles/db.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : db/CMakeFiles/db.dir/depend

