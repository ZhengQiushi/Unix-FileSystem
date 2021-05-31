cmake_minimum_required(VERSION 2.8)


set(FLATFORM "LINUX") # WINDOWS


project(my_VFS)

include_directories(include)


if("${FLATFORM}" STREQUAL "WINDOWS")
    message(STATUS " Compiling for Windows")
    # Sample toolchain file for building for Windows from an Ubuntu Linux system.
    #
    # Typical usage:
    #    *) install cross compiler: `sudo apt-get install mingw-w64`
    #    *) cd build
    #    *) cmake -DCMAKE_TOOLCHAIN_FILE=~/mingw-w64-x86_64.cmake ..

    set(CMAKE_SYSTEM_NAME Windows)
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

    # cross compilers to use for C, C++ and Fortran
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_Fortran_COMPILER ${TOOLCHAIN_PREFIX}-gfortran)
    set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

    # target environment on the build host system
    set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

    # modify default behavior of FIND_XXX() commands
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive -Wl,-Bdynamic")

    include_directories(/mingw/include)
    
    add_executable(my_VFS  main.cpp src/Shell.cpp src/Kernel.cpp src/BufferCache.cpp src/DiskDriver.cpp src/FileSystem.cpp 
                                src/Tools.cpp src/VFS.cpp)
    target_link_libraries(my_VFS /mingw/lib/libmman.a)

else ()
    message(STATUS " Compiling for Linux")
    add_executable(my_VFS  main.cpp src/Shell.cpp src/Kernel.cpp src/BufferCache.cpp src/DiskDriver.cpp src/FileSystem.cpp 
    src/Tools.cpp)

endif()
