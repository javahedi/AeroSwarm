#pragma once

#include "aeroswarm/app/scenario.hpp"

Scenario make_random_scenario(
    int width,
    int height,
    int obstacle_count,
    unsigned int seed
);