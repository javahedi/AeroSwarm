#include <iostream>
#include <vector>

#include "aeroswarm/app/sequential_runner.hpp"
#include "aeroswarm/drone.hpp"
#include "aeroswarm/sequential/terrain.hpp"
#include "aeroswarm/sequential/simulation.hpp"


int run_sequential(const Scenario& scenario) {
    Terrain terrain{
        scenario.width,
        scenario.height
    };

    terrain.set_target(scenario.target);
    for (const auto& obstacle : scenario.obstacles) {
        terrain.set_obstacle(obstacle);
    }

    Simulation simulation{
        terrain,
        scenario.drones,
        scenario.seed
    };

    const auto status = simulation.run_until_done();

    if (status == SimulationStatus::TargetFound) {
        std::cout << "Sequential simulation: target found\n";

        const auto winner = simulation.winning_drone_id();

        if (winner.has_value()) {
            std::cout << "Winning drone: #" << winner.value() << '\n';
        }

        return 0;
    }

    std::cout << "Sequential simulation: stuck\n";
    return 0;
}