#include <iostream>
#include <vector>

#include "aeroswarm/app/parallel_runner.hpp"
#include "aeroswarm/drone.hpp"
#include "aeroswarm/parallel/terrain.hpp"
#include "aeroswarm/parallel/simulation.hpp" 

int run_parallel(const Scenario& scenario) {
    ParallelTerrain terrain{
        scenario.width,
        scenario.height
    };

    terrain.set_target(scenario.target);
    for (const auto& obstacle : scenario.obstacles) {
        terrain.set_obstacle(obstacle);
    }

    ParallelSimulation simulation{
        terrain,
        scenario.drones,
        scenario.seed
    };

    const auto status = simulation.run();

    if (status == ParallelSimulationStatus::TargetFound) {
        std::cout << "Parallel simulation: target found\n";

        const auto winner = simulation.winning_drone_id();

        if (winner.has_value()) {
            std::cout << "Winning drone: #" << winner.value() << '\n';
        }

        return 0;
    }

    std::cout << "Parallel simulation: stuck\n";
    return 0;
}