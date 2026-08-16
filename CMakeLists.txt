cmake_minimum_required(VERSION 3.10)
project(AeroSwarm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(SequentialCore STATIC
    src/sequential_simulation.cpp
)

add_library(ParallelCore STATIC
    src/parallel_simulation.cpp
)

add_executable(AeroSwarm
    src/main.cpp
)

target_link_libraries(AeroSwarm PRIVATE
    SequentialCore
    ParallelCore
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
    ${CMAKE_CURRENT_BINARY_DIR}/config.txt
    COPYONLY
)

target_compile_options(SequentialCore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_options(ParallelCore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_options(AeroSwarm PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()

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

    add_executable(tests
        tests/test_terrain.cpp
        tests/test_drone_baseline.cpp
        tests/test_sequential_simulation.cpp
        tests/test_parallel_terrain.cpp
        tests/test_parallel_simulation.cpp
        tests/test_comparison.cpp
    )

    target_link_libraries(tests PRIVATE
        SequentialCore
        ParallelCore
        Catch2::Catch2WithMain
    )

    target_include_directories(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    include(CTest)
    include(Catch)

    catch_discover_tests(tests)
endif()