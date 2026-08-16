#include <iostream>
#include <string>

#include "aeroswarm/app/sequential_runner.hpp"
#include "aeroswarm/app/parallel_runner.hpp"
#include "aeroswarm/app/scenario_validation.hpp"
#include "aeroswarm/app/parallel_live_runner.hpp"
#include "aeroswarm/app/parallel_sdl_runner.hpp"
//#include "aeroswarm/app/scenario.hpp"
#include "aeroswarm/app/scenario_factory.hpp"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout
            << "Usage: "
            << argv[0]
            << " <sequential|parallel|parallel-live|parallel-sdl>\n";
        return 1;
    }

    const std::string mode = argv[1];


    Scenario scenario =
        make_random_scenario(
            30,   // width
            30,   // height
            80,   // random obstacles
            42    // reproducible seed

        );
    std::string error_message;

    if (!validate_scenario(scenario, error_message)) {
        std::cerr << "Invalid scenario: "
                << error_message
                << '\n';

        return 1;
    }

    if (mode == "sequential") {
        return run_sequential(scenario);
    }

    if (mode == "parallel") {
        return run_parallel(scenario);
    }

    if (mode == "parallel-live") {
        return run_parallel_live(scenario);
    }

    if (mode == "parallel-sdl") {
        return run_parallel_sdl(scenario);
    }

    std::cerr << "Unknown mode: " << mode << '\n';
    return 1;

    

    return 0;
}

/*
cmake -S . -B build     ← CONFIGURE / create build directory
cmake --build build     ← BUILD using that directory
ctest --test-dir build  ← TEST what was built

1: 
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure

2: 
cmake -S . -B build-tsan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"

cmake --build build-tsan
ctest --test-dir build-tsan --output-on-failure
*/