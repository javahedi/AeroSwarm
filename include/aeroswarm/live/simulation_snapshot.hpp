#pragma once

#include <optional>
#include <vector>

#include "aeroswarm/types.hpp"

struct SimulationSnapshot {
    std::vector<Position> drone_positions;
    std::vector<Position> visited_cells;

    Position target{};

    bool target_found{false};
    std::optional<int> winning_drone_id;

    std::size_t tick{0};
};