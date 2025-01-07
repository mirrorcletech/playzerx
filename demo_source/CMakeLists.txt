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

add_custom_command(
    TARGET PlayzerX-Demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/butterfly.smp
            $<TARGET_FILE_DIR:PlayzerX-Demo>/butterfly.smp
)

# Require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add directory for libary header files
target_include_directories( PlayzerX-Demo PRIVATE ../include )
target_include_directories( PlayzerX-Demo PRIVATE ../mtidevice/include )


# Link the required libraries
target_link_libraries(PlayzerX-Demo PlayzerX)