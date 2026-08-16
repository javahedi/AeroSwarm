cmake_minimum_required(VERSION 3.10)

project(AeroSwarm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)


# ============================================================
# SDL3
# ============================================================
find_package(SDL3 REQUIRED CONFIG)


# ============================================================
# Sequential simulation core
# ============================================================

add_library(SequentialCore STATIC
    src/sequential_simulation.cpp
)

target_compile_options(SequentialCore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)


# ============================================================
# Parallel simulation core
# ============================================================

add_library(ParallelCore STATIC
    src/parallel_simulation.cpp
)

target_compile_options(ParallelCore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)


# ============================================================
# Application layer
# ============================================================

add_library(AppCore STATIC
    src/sequential_runner.cpp
    src/parallel_runner.cpp
    src/parallel_live_runner.cpp
    src/parallel_sdl_runner.cpp
    src/scenario_validation.cpp
    src/scenario_factory.cpp
    src/sdl_renderer.cpp
)


target_link_libraries(AppCore PUBLIC
    SequentialCore
    ParallelCore
    SDL3::SDL3
)

target_compile_options(AppCore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)


# ============================================================
# Main executable
# ============================================================

add_executable(AeroSwarm
    src/main.cpp
)

target_link_libraries(AeroSwarm PRIVATE
    AppCore
)

target_compile_options(AeroSwarm PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)


# ============================================================
# Runtime output
# ============================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)


# ============================================================
# Config file
# ============================================================

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.txt
    ${CMAKE_CURRENT_BINARY_DIR}/config.txt
    COPYONLY
)



# ============================================================
# Tests
# ============================================================

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)

    enable_testing()

    find_package(Catch2 QUIET)

    if(NOT Catch2_FOUND)

        message(
            STATUS
            "Catch2 not found, downloading with FetchContent..."
        )

        include(FetchContent)

        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.5.2
        )

        FetchContent_MakeAvailable(Catch2)

    else()

        message(
            STATUS
            "Using system-installed Catch2"
        )

    endif()


    add_executable(tests
        tests/test_drone_baseline.cpp
        tests/test_sequential_terrain.cpp
        tests/test_sequential_simulation.cpp
        tests/test_parallel_terrain.cpp
        tests/test_parallel_simulation.cpp
        tests/test_comparison.cpp
        tests/test_scenario_validation.cpp
        tests/test_scenario_factory.cpp
    )


    target_link_libraries(tests PRIVATE
        SequentialCore
        ParallelCore
        AppCore
        Catch2::Catch2WithMain
    )


    target_include_directories(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )


    target_compile_options(tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )


    include(CTest)
    include(Catch)

    catch_discover_tests(tests)

endif()