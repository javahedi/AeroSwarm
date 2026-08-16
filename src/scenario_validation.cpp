#include "aeroswarm/app/scenario_validation.hpp"

// the anonymous namespace gives it internal linkage, 
// so it is not part of your public application API.
namespace { 

bool in_bounds(const Position& pos, int width, int height) {
    return pos.x >= 0 &&
           pos.x < width &&
           pos.y >= 0 &&
           pos.y < height;
}

} // namespace

bool validate_scenario(
    const Scenario& scenario,
    std::string& error_message)
{
    if (scenario.width <= 0 || scenario.height <= 0) {
        error_message = "Terrain dimensions must be positive";
        return false;
    }

    if (!in_bounds(
            scenario.target,
            scenario.width,
            scenario.height)) {
        error_message = "Target position is outside terrain bounds";
        return false;
    }

    for (const auto& obstacle : scenario.obstacles) {
        if (!in_bounds(
                obstacle,
                scenario.width,
                scenario.height)) {
            error_message = "Obstacle position is outside terrain bounds";
            return false;
        }

        if (obstacle == scenario.target) {
            error_message = "Obstacle cannot overlap target";
            return false;
        }
    }

    for (const auto& drone : scenario.drones) {
        const Position& start = drone.position();

        if (!in_bounds(
                start,
                scenario.width,
                scenario.height)) {
            error_message = "Drone start position is outside terrain bounds";
            return false;
        }

        for (const auto& obstacle : scenario.obstacles) {
            if (start == obstacle) {
                error_message = "Drone cannot start on an obstacle";
                return false;
            }
        }
    }

    error_message.clear();
    return true;
}