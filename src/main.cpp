#include <iostream>
#include <string>
#include <vector>

#include "aeroswarm/drone.hpp"
#include "aeroswarm/terrain.hpp"
#include "aeroswarm/sequential_simulation.hpp"

#include "aeroswarm/parallel/parallel_terrain.hpp"
#include "aeroswarm/parallel/parallel_simulation.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <sequential|parallel>\n";
        return 1;
    }

    const std::string mode = argv[1];

    if (mode == "sequential") {
        std::cout << "Running sequential simulation\n";
    } else if (mode == "parallel") {
        std::cout << "Running parallel simulation\n";
    } else {
        std::cout << "Unknown mode\n";
        return 1;
    }

    

    return 0;
}