cmake_minimum_required(VERSION 3.10)
project(AeroSwarm CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/terrain.cpp
    src/run.cpp
    src/drone.cpp
    src/utility.cpp
)

# Core static library
add_library(SwarmCore STATIC
    src/terrain.cpp
    src/run.cpp
    src/drone.cpp
    src/utility.cpp
)

# Main executable
add_executable(AeroSwarm src/main.cpp)
target_link_libraries(AeroSwarm PRIVATE SwarmCore)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Configure file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
    ${CMAKE_CURRENT_BINARY_DIR}/config.txt
    COPYONLY
)

# Link stdc++fs if necessary (older GCC)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(AeroSwarm PRIVATE stdc++fs)
    target_link_libraries(SwarmCore PRIVATE stdc++fs)
endif()

# Compiler warnings and flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(AeroSwarm PRIVATE -Wall -Wextra -Wpedantic -O3)
    target_compile_options(SwarmCore PRIVATE -Wall -Wextra -Wpedantic -O3)
else()
    target_compile_options(AeroSwarm PRIVATE -Wall -Wextra -Wpedantic -g)
    target_compile_options(SwarmCore PRIVATE -Wall -Wextra -Wpedantic -g)
endif()

# Tests
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()

    # Try to find system Catch2 first
    find_package(Catch2 QUIET)

    if(NOT Catch2_FOUND)
        message(STATUS "Catch2 not found, downloading with FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.5.2
        )
        FetchContent_MakeAvailable(Catch2)
    else()
        message(STATUS "Using system-installed Catch2")
    endif()

    add_executable(tests tests/test_drone.cpp)
    target_link_libraries(tests PRIVATE SwarmCore Catch2::Catch2WithMain)
    target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

    include(CTest)
    include(Catch)
    catch_discover_tests(tests)
endif()
