# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.27.5/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.27.5/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/administrator/Documents/source/minminvk/MinminVk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/administrator/Documents/source/minminvk/MinminVk/build

# Include any dependencies generated for this target.
include external/glfw/tests/CMakeFiles/empty.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include external/glfw/tests/CMakeFiles/empty.dir/compiler_depend.make

# Include the progress variables for this target.
include external/glfw/tests/CMakeFiles/empty.dir/progress.make

# Include the compile flags for this target's objects.
include external/glfw/tests/CMakeFiles/empty.dir/flags.make

external/glfw/tests/CMakeFiles/empty.dir/empty.c.o: external/glfw/tests/CMakeFiles/empty.dir/flags.make
external/glfw/tests/CMakeFiles/empty.dir/empty.c.o: /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/tests/empty.c
external/glfw/tests/CMakeFiles/empty.dir/empty.c.o: external/glfw/tests/CMakeFiles/empty.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/administrator/Documents/source/minminvk/MinminVk/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object external/glfw/tests/CMakeFiles/empty.dir/empty.c.o"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT external/glfw/tests/CMakeFiles/empty.dir/empty.c.o -MF CMakeFiles/empty.dir/empty.c.o.d -o CMakeFiles/empty.dir/empty.c.o -c /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/tests/empty.c

external/glfw/tests/CMakeFiles/empty.dir/empty.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/empty.dir/empty.c.i"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/tests/empty.c > CMakeFiles/empty.dir/empty.c.i

external/glfw/tests/CMakeFiles/empty.dir/empty.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/empty.dir/empty.c.s"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/tests/empty.c -o CMakeFiles/empty.dir/empty.c.s

external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.o: external/glfw/tests/CMakeFiles/empty.dir/flags.make
external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.o: /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/tinycthread.c
external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.o: external/glfw/tests/CMakeFiles/empty.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/administrator/Documents/source/minminvk/MinminVk/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.o"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.o -MF CMakeFiles/empty.dir/__/deps/tinycthread.c.o.d -o CMakeFiles/empty.dir/__/deps/tinycthread.c.o -c /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/tinycthread.c

external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/empty.dir/__/deps/tinycthread.c.i"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/tinycthread.c > CMakeFiles/empty.dir/__/deps/tinycthread.c.i

external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/empty.dir/__/deps/tinycthread.c.s"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/tinycthread.c -o CMakeFiles/empty.dir/__/deps/tinycthread.c.s

external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.o: external/glfw/tests/CMakeFiles/empty.dir/flags.make
external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.o: /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/glad_gl.c
external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.o: external/glfw/tests/CMakeFiles/empty.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/administrator/Documents/source/minminvk/MinminVk/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.o"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.o -MF CMakeFiles/empty.dir/__/deps/glad_gl.c.o.d -o CMakeFiles/empty.dir/__/deps/glad_gl.c.o -c /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/glad_gl.c

external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/empty.dir/__/deps/glad_gl.c.i"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/glad_gl.c > CMakeFiles/empty.dir/__/deps/glad_gl.c.i

external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/empty.dir/__/deps/glad_gl.c.s"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/deps/glad_gl.c -o CMakeFiles/empty.dir/__/deps/glad_gl.c.s

# Object files for target empty
empty_OBJECTS = \
"CMakeFiles/empty.dir/empty.c.o" \
"CMakeFiles/empty.dir/__/deps/tinycthread.c.o" \
"CMakeFiles/empty.dir/__/deps/glad_gl.c.o"

# External object files for target empty
empty_EXTERNAL_OBJECTS =

external/glfw/tests/empty.app/Contents/MacOS/empty: external/glfw/tests/CMakeFiles/empty.dir/empty.c.o
external/glfw/tests/empty.app/Contents/MacOS/empty: external/glfw/tests/CMakeFiles/empty.dir/__/deps/tinycthread.c.o
external/glfw/tests/empty.app/Contents/MacOS/empty: external/glfw/tests/CMakeFiles/empty.dir/__/deps/glad_gl.c.o
external/glfw/tests/empty.app/Contents/MacOS/empty: external/glfw/tests/CMakeFiles/empty.dir/build.make
external/glfw/tests/empty.app/Contents/MacOS/empty: external/glfw/src/libglfw3.a
external/glfw/tests/empty.app/Contents/MacOS/empty: external/glfw/tests/CMakeFiles/empty.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/administrator/Documents/source/minminvk/MinminVk/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable empty.app/Contents/MacOS/empty"
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/empty.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
external/glfw/tests/CMakeFiles/empty.dir/build: external/glfw/tests/empty.app/Contents/MacOS/empty
.PHONY : external/glfw/tests/CMakeFiles/empty.dir/build

external/glfw/tests/CMakeFiles/empty.dir/clean:
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests && $(CMAKE_COMMAND) -P CMakeFiles/empty.dir/cmake_clean.cmake
.PHONY : external/glfw/tests/CMakeFiles/empty.dir/clean

external/glfw/tests/CMakeFiles/empty.dir/depend:
	cd /Users/administrator/Documents/source/minminvk/MinminVk/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/administrator/Documents/source/minminvk/MinminVk /Users/administrator/Documents/source/minminvk/MinminVk/external/glfw/tests /Users/administrator/Documents/source/minminvk/MinminVk/build /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests /Users/administrator/Documents/source/minminvk/MinminVk/build/external/glfw/tests/CMakeFiles/empty.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : external/glfw/tests/CMakeFiles/empty.dir/depend

