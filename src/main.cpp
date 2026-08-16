#include <iostream>
#include <string>
#include <vector>

#include "aeroswarm/drone.hpp"
#include "aeroswarm/terrain.hpp"
#include "aeroswarm/sequential_simulation.hpp"

#include "aeroswarm/parallel/parallel_terrain.hpp"
#include "aeroswarm/parallel/parallel_simulation.hpp"


#include "aeroswarm/app/sequential_runner.hpp"
#include "aeroswarm/app/parallel_runner.hpp"
#include "aeroswarm/app/scenario.hpp"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <sequential|parallel>\n";
        return 1;
    }

    const std::string mode = argv[1];


    Scenario scenario;

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