# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# Make sure CMake finds library file in installations without PlayzerX source
link_directories(../lib)

# Define the executable
add_executable(
    # Executable name
    PlayzerX-Demo

    # Source files
    PlayzerX-Demo.cpp )
    
# Require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add directory for libary header files
target_include_directories( PlayzerX-Demo PRIVATE ../include )
target_include_directories( PlayzerX-Demo PRIVATE ../mtidevice/include )


# Link the required libraries
target_link_libraries(PlayzerX-Demo PlayzerX)