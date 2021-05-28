cmake_minimum_required(VERSION 2.8)

project(my_VFS)

include_directories(include)

add_executable(my_VFS  main.cpp src/Shell.cpp src/Kernel.cpp src/BufferCache.cpp src/DiskDriver.cpp src/FileSystem.cpp 
                               src/Tools.cpp src/VFS.cpp )
