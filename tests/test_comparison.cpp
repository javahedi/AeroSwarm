#include <catch2/catch_test_macros.hpp>


#include "aeroswarm/sequential_simulation.hpp"
#include "aeroswarm/parallel/parallel_simulation.hpp"

TEST_CASE("Sequential and parallel simulations both find a reachable target") {
    Terrain sequential_terrain{3, 3};
    sequential_terrain.set_target({2, 1});
    sequential_terrain.set_obstacle({0, 1});
    sequential_terrain.set_obstacle({1, 0});
    sequential_terrain.set_obstacle({1, 2});

    Simulation sequential{
        sequential_terrain,
        {Drone{7, {1, 1}}},
        42
    };

    ParallelTerrain parallel_terrain{3, 3};
    parallel_terrain.set_target({2, 1});
    parallel_terrain.set_obstacle({0, 1});
    parallel_terrain.set_obstacle({1, 0});
    parallel_terrain.set_obstacle({1, 2});

    ParallelSimulation parallel{
        parallel_terrain,
        {Drone{7, {1, 1}}},
        42
    };

    const auto sequential_status = sequential.run_until_done();
    const auto parallel_status = parallel.run();

    REQUIRE(sequential_status == SimulationStatus::TargetFound);
    REQUIRE(parallel_status == ParallelSimulationStatus::TargetFound);

    REQUIRE(sequential.winning_drone_id().has_value());
    REQUIRE(parallel.winning_drone_id().has_value());

    REQUIRE(sequential.winning_drone_id().value() == 7);
    REQUIRE(parallel.winning_drone_id().value() == 7);
}


TEST_CASE("Sequential and parallel simulations both report stuck") {
    Terrain sequential_terrain{3, 3};
    sequential_terrain.set_obstacle({0, 1});
    sequential_terrain.set_obstacle({2, 1});
    sequential_terrain.set_obstacle({1, 0});
    sequential_terrain.set_obstacle({1, 2});

    Simulation sequential{
        sequential_terrain,
        {Drone{1, {1, 1}}},
        42
    };

    ParallelTerrain parallel_terrain{3, 3};
    parallel_terrain.set_obstacle({0, 1});
    parallel_terrain.set_obstacle({2, 1});
    parallel_terrain.set_obstacle({1, 0});
    parallel_terrain.set_obstacle({1, 2});

    ParallelSimulation parallel{
        parallel_terrain,
        {Drone{1, {1, 1}}},
        42
    };

    const auto sequential_status = sequential.run_until_done();
    const auto parallel_status = parallel.run();

    REQUIRE(sequential_status == SimulationStatus::Stuck);
    REQUIRE(parallel_status == ParallelSimulationStatus::Stuck);

    REQUIRE_FALSE(sequential.target_found());
    REQUIRE_FALSE(parallel.target_found());
}