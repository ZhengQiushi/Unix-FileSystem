# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_SOURCE_DIR = /home/truth/Desktop/hai/Unix-FileSystem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/truth/Desktop/hai/Unix-FileSystem/build

# Include any dependencies generated for this target.
include CMakeFiles/my_VFS.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/my_VFS.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/my_VFS.dir/flags.make

CMakeFiles/my_VFS.dir/main.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/my_VFS.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/main.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/main.cpp

CMakeFiles/my_VFS.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/main.cpp > CMakeFiles/my_VFS.dir/main.cpp.i

CMakeFiles/my_VFS.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/main.cpp -o CMakeFiles/my_VFS.dir/main.cpp.s

CMakeFiles/my_VFS.dir/src/Shell.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/Shell.cpp.o: ../src/Shell.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/my_VFS.dir/src/Shell.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/Shell.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/Shell.cpp

CMakeFiles/my_VFS.dir/src/Shell.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/Shell.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/Shell.cpp > CMakeFiles/my_VFS.dir/src/Shell.cpp.i

CMakeFiles/my_VFS.dir/src/Shell.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/Shell.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/Shell.cpp -o CMakeFiles/my_VFS.dir/src/Shell.cpp.s

CMakeFiles/my_VFS.dir/src/Kernel.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/Kernel.cpp.o: ../src/Kernel.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/my_VFS.dir/src/Kernel.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/Kernel.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/Kernel.cpp

CMakeFiles/my_VFS.dir/src/Kernel.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/Kernel.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/Kernel.cpp > CMakeFiles/my_VFS.dir/src/Kernel.cpp.i

CMakeFiles/my_VFS.dir/src/Kernel.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/Kernel.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/Kernel.cpp -o CMakeFiles/my_VFS.dir/src/Kernel.cpp.s

CMakeFiles/my_VFS.dir/src/BufferCache.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/BufferCache.cpp.o: ../src/BufferCache.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/my_VFS.dir/src/BufferCache.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/BufferCache.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/BufferCache.cpp

CMakeFiles/my_VFS.dir/src/BufferCache.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/BufferCache.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/BufferCache.cpp > CMakeFiles/my_VFS.dir/src/BufferCache.cpp.i

CMakeFiles/my_VFS.dir/src/BufferCache.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/BufferCache.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/BufferCache.cpp -o CMakeFiles/my_VFS.dir/src/BufferCache.cpp.s

CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.o: ../src/DiskDriver.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/DiskDriver.cpp

CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/DiskDriver.cpp > CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.i

CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/DiskDriver.cpp -o CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.s

CMakeFiles/my_VFS.dir/src/FileSystem.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/FileSystem.cpp.o: ../src/FileSystem.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/my_VFS.dir/src/FileSystem.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/FileSystem.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/FileSystem.cpp

CMakeFiles/my_VFS.dir/src/FileSystem.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/FileSystem.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/FileSystem.cpp > CMakeFiles/my_VFS.dir/src/FileSystem.cpp.i

CMakeFiles/my_VFS.dir/src/FileSystem.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/FileSystem.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/FileSystem.cpp -o CMakeFiles/my_VFS.dir/src/FileSystem.cpp.s

CMakeFiles/my_VFS.dir/src/Tools.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/Tools.cpp.o: ../src/Tools.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/my_VFS.dir/src/Tools.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/Tools.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/Tools.cpp

CMakeFiles/my_VFS.dir/src/Tools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/Tools.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/Tools.cpp > CMakeFiles/my_VFS.dir/src/Tools.cpp.i

CMakeFiles/my_VFS.dir/src/Tools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/Tools.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/Tools.cpp -o CMakeFiles/my_VFS.dir/src/Tools.cpp.s

CMakeFiles/my_VFS.dir/src/VFS.cpp.o: CMakeFiles/my_VFS.dir/flags.make
CMakeFiles/my_VFS.dir/src/VFS.cpp.o: ../src/VFS.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/my_VFS.dir/src/VFS.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/my_VFS.dir/src/VFS.cpp.o -c /home/truth/Desktop/hai/Unix-FileSystem/src/VFS.cpp

CMakeFiles/my_VFS.dir/src/VFS.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/my_VFS.dir/src/VFS.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/truth/Desktop/hai/Unix-FileSystem/src/VFS.cpp > CMakeFiles/my_VFS.dir/src/VFS.cpp.i

CMakeFiles/my_VFS.dir/src/VFS.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/my_VFS.dir/src/VFS.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/truth/Desktop/hai/Unix-FileSystem/src/VFS.cpp -o CMakeFiles/my_VFS.dir/src/VFS.cpp.s

# Object files for target my_VFS
my_VFS_OBJECTS = \
"CMakeFiles/my_VFS.dir/main.cpp.o" \
"CMakeFiles/my_VFS.dir/src/Shell.cpp.o" \
"CMakeFiles/my_VFS.dir/src/Kernel.cpp.o" \
"CMakeFiles/my_VFS.dir/src/BufferCache.cpp.o" \
"CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.o" \
"CMakeFiles/my_VFS.dir/src/FileSystem.cpp.o" \
"CMakeFiles/my_VFS.dir/src/Tools.cpp.o" \
"CMakeFiles/my_VFS.dir/src/VFS.cpp.o"

# External object files for target my_VFS
my_VFS_EXTERNAL_OBJECTS =

my_VFS: CMakeFiles/my_VFS.dir/main.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/Shell.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/Kernel.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/BufferCache.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/DiskDriver.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/FileSystem.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/Tools.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/src/VFS.cpp.o
my_VFS: CMakeFiles/my_VFS.dir/build.make
my_VFS: CMakeFiles/my_VFS.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX executable my_VFS"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/my_VFS.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/my_VFS.dir/build: my_VFS

.PHONY : CMakeFiles/my_VFS.dir/build

CMakeFiles/my_VFS.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/my_VFS.dir/cmake_clean.cmake
.PHONY : CMakeFiles/my_VFS.dir/clean

CMakeFiles/my_VFS.dir/depend:
	cd /home/truth/Desktop/hai/Unix-FileSystem/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/truth/Desktop/hai/Unix-FileSystem /home/truth/Desktop/hai/Unix-FileSystem /home/truth/Desktop/hai/Unix-FileSystem/build /home/truth/Desktop/hai/Unix-FileSystem/build /home/truth/Desktop/hai/Unix-FileSystem/build/CMakeFiles/my_VFS.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/my_VFS.dir/depend

