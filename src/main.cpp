#include <iostream>
#include <string>

#include "aeroswarm/app/sequential_runner.hpp"
#include "aeroswarm/app/parallel_runner.hpp"
#include "aeroswarm/app/scenario.hpp"
#include "aeroswarm/app/scenario_validation.hpp"


int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <sequential|parallel>\n";
        return 1;
    }

    const std::string mode = argv[1];


    Scenario scenario;
    std::string error_message;

    if (!validate_scenario(scenario, error_message)) {
        std::cerr << "Invalid scenario: "
                << error_message
                << '\n';

        return 1;
    }

    if (mode == "sequential") {
        return run_sequential(scenario);
    } else if (mode == "parallel") {
        return run_parallel(scenario);
    } else {
        std::cout << "Unknown mode\n";
        return 1;
    }

    

    return 0;
}