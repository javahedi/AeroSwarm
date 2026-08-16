#pragma once

#include <vector>

#include "aeroswarm/drone.hpp"
#include "aeroswarm/types.hpp"

struct Scenario {
    int width{30};
    int height{30};

    Position target{10, 20};

    std::vector<Position> obstacles{
        {2, 1},
        {2, 2},
        {2, 3}
    };

    std::vector<Drone> drones{
        Drone{1, {0, 0}}
    };

    unsigned int seed{42};
};