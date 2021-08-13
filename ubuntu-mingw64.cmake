# Toolchain file for building for Windows from an Ubuntu Linux system.
#
# use -DCMAKE_TOOLCHAIN_FILE=ubuntu-mingw64.cmake
#

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX i686-w64-mingw32)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-c++-posix)

# target environment on the build host system
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX} /usr/lib/gcc/${TOOLCHAIN_PREFIX}/9.3-posix)
set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES
        /usr/lib/gcc/${TOOLCHAIN_PREFIX}/9.3-posix)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

