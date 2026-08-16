#pragma once

#include <optional>
#include <vector>
#include <optional>
#include "aeroswarm/types.hpp"

struct SimulationSnapshot {
    std::vector<Position> drone_positions;
    std::vector<Position> visited_cells;
    std::vector<Position> obstacle_positions;
    std::optional<Position> target;

    //Position target{};

    bool target_found{false};
    std::optional<int> winning_drone_id;

    std::size_t tick{0};
};