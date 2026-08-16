#include "aeroswarm/app/scenario_factory.hpp"

#include <random>
#include <vector>

namespace {

bool contains_position(
    const std::vector<Position>& positions,
    const Position& candidate)
{
    for (const auto& pos : positions) {
        if (pos == candidate) {
            return true;
        }
    }

    return false;
}

bool is_drone_start(
    const std::vector<Drone>& drones,
    const Position& candidate)
{
    for (const auto& drone : drones) {
        if (drone.position() == candidate) {
            return true;
        }
    }

    return false;
}

} // namespace


Scenario make_random_scenario(
    int width,
    int height,
    int obstacle_count,
    unsigned int seed)
{
    Scenario scenario;

    scenario.width = width;
    scenario.height = height;
    scenario.seed = seed;

    // Four drones start from the four corners.
    scenario.drones = {
        Drone{1, {0, 0}},
        Drone{2, {width - 1, 0}},
        Drone{3, {0, height - 1}},
        Drone{4, {width - 1, height - 1}}
    };

    std::mt19937 rng(seed);

    std::uniform_int_distribution<int> x_dist(
        0,
        width - 1
    );

    std::uniform_int_distribution<int> y_dist(
        0,
        height - 1
    );

    // Random target, but never on a drone starting cell.
    while (true) {
        Position candidate{
            x_dist(rng),
            y_dist(rng)
        };

        if (!is_drone_start(scenario.drones, candidate)) {
            scenario.target = candidate;
            break;
        }
    }

    // Random unique obstacles.
    scenario.obstacles.clear();

    while (
        static_cast<int>(scenario.obstacles.size())
        < obstacle_count)
    {
        Position candidate{
            x_dist(rng),
            y_dist(rng)
        };

        // Never put an obstacle on the target.
        if (candidate == scenario.target) {
            continue;
        }

        // Never block a drone's starting position.
        if (is_drone_start(scenario.drones, candidate)) {
            continue;
        }

        // Avoid duplicate obstacles.
        if (contains_position(
                scenario.obstacles,
                candidate)) {
            continue;
        }

        scenario.obstacles.push_back(candidate);
    }

    return scenario;
}