#include <catch2/catch_test_macros.hpp>

#include "aeroswarm/parallel/parallel_simulation.hpp"

TEST_CASE("ParallelSimulation initializes without a winner") {
    ParallelTerrain terrain{3, 3};

    std::vector<Drone> drones{
        Drone{1, {0, 0}},
        Drone{2, {2, 2}}
    };

    ParallelSimulation simulation{
        terrain,
        drones,
        42
    };

    REQUIRE_FALSE(simulation.target_found());
    REQUIRE_FALSE(simulation.winning_drone_id().has_value());
}


TEST_CASE("ParallelSimulation run returns without hanging") {
    ParallelTerrain terrain{3, 3};

    std::vector<Drone> drones{
        Drone{1, {0, 0}},
        Drone{2, {2, 2}}
    };

    ParallelSimulation simulation{
        terrain,
        drones,
        42
    };

    simulation.run();

    REQUIRE_FALSE(simulation.target_found());
    REQUIRE_FALSE(simulation.winning_drone_id().has_value());
}

// 
TEST_CASE("ParallelSimulation detects target and records winning drone (deterministic)") {
    ParallelTerrain terrain{3, 3};

    terrain.set_target({2, 1});

    terrain.set_obstacle({0, 1});
    terrain.set_obstacle({1, 0});
    terrain.set_obstacle({1, 2});

    std::vector<Drone> drones{
        Drone{7, {1, 1}}
    };

    ParallelSimulation simulation{
        terrain,
        drones,
        42
    };

    simulation.run();

    REQUIRE(simulation.target_found());

    const auto winner = simulation.winning_drone_id();

    REQUIRE(winner.has_value());
    REQUIRE(winner.value() == 7);
}