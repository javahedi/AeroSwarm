cmake_minimum_required(VERSION 3.10)
project(SwarmSimulator CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Specify include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Specify source files
set(SOURCES
    src/main.cpp
    src/terrain.cpp
    src/run.cpp
    src/drone.cpp
    src/utility.cpp
)

# Create executable
add_executable(SwarmSimulator ${SOURCES})

# Conditionally link stdc++fs for older GCC versions
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(SwarmSimulator PRIVATE stdc++fs)
endif()

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable warnings and optimizations based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(SwarmSimulator PRIVATE -Wall -Wextra -Wpedantic -O3)
else()
    target_compile_options(SwarmSimulator PRIVATE -Wall -Wextra -Wpedantic -g)
endif()