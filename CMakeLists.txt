# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# Cmake file for making the PlayzerX library
project( PlayzerX )

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you
add_library( # Library name
             PlayzerX

             # Library type (SHARED or STATIC)
             SHARED

             # Source files
             PlayzerX.cpp
             MTISerial.cpp)

# Require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add directory for libary header files
target_include_directories( PlayzerX PRIVATE include )
target_include_directories( PlayzerX PRIVATE mtidevice/include )

target_link_libraries( PlayzerX ${CMAKE_THREAD_LIBS_INIT} )

# Build the customer demo app as well
add_subdirectory(demo_source)