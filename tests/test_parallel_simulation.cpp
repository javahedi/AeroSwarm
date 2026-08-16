#include <catch2/catch_test_macros.hpp>
#include "aeroswarm/parallel/simulation.hpp"

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
TEST_CASE("ParallelSimulation detects target and records winning drone") {
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

    const auto status = simulation.run();

    REQUIRE(status == ParallelSimulationStatus::TargetFound);
    REQUIRE(simulation.target_found());

    const auto winner = simulation.winning_drone_id();

    REQUIRE(winner.has_value());
    REQUIRE(winner.value() == 7);
}



TEST_CASE("ParallelSimulation reports stuck when drone cannot move") {
    ParallelTerrain terrain{3, 3};

    terrain.set_obstacle({0, 1});
    terrain.set_obstacle({2, 1});
    terrain.set_obstacle({1, 0});
    terrain.set_obstacle({1, 2});

    ParallelSimulation simulation{
        terrain,
        {Drone{1, {1, 1}}},
        42
    };

    const auto status = simulation.run();

    REQUIRE(status == ParallelSimulationStatus::Stuck);
    REQUIRE_FALSE(simulation.target_found());
    REQUIRE_FALSE(simulation.winning_drone_id().has_value());
}



TEST_CASE("ParallelSimulation allows drones to share a starting position") {
    ParallelTerrain terrain{3, 3};

    std::vector<Drone> drones{
        Drone{1, {1, 1}},
        Drone{2, {1, 1}}
    };

    REQUIRE_NOTHROW(
        ParallelSimulation{terrain, drones, 42}
    );
}


TEST_CASE("ParallelSimulation rejects drone starting on obstacle") {
    ParallelTerrain terrain{3, 3};

    terrain.set_obstacle({1, 1});

    std::vector<Drone> drones{
        Drone{1, {1, 1}}
    };

    REQUIRE_THROWS_AS(
        ParallelSimulation(terrain, drones, 42),
        std::invalid_argument
    );
}

TEST_CASE("ParallelSimulation rejects out-of-bounds drone start") {
    ParallelTerrain terrain{3, 3};

    std::vector<Drone> drones{
        Drone{1, {10, 10}}
    };

    REQUIRE_THROWS_AS(
        ParallelSimulation(terrain, drones, 42),
        std::invalid_argument
    );
}




TEST_CASE("ParallelSimulation terminates with many competing drones") {
    ParallelTerrain terrain{10, 10};

    terrain.set_target({9, 9});

    std::vector<Drone> drones;

    for (int id = 0; id < 20; ++id) {
        drones.emplace_back(id, Position{0, 0});
    }

    ParallelSimulation simulation{
        terrain,
        drones,
        42
    };

    const auto status = simulation.run();

    const bool valid_status =
        status == ParallelSimulationStatus::TargetFound ||
        status == ParallelSimulationStatus::Stuck;

    REQUIRE(valid_status);

    if (status == ParallelSimulationStatus::TargetFound) {
        REQUIRE(simulation.target_found());
        REQUIRE(simulation.winning_drone_id().has_value());
    }

    if (status == ParallelSimulationStatus::Stuck) {
        REQUIRE_FALSE(simulation.target_found());
        REQUIRE_FALSE(simulation.winning_drone_id().has_value());
    }
}

/*
for i in {1..100}; do
    ctest --test-dir build --output-on-failure || break
done
*/