#include <catch2/catch_test_macros.hpp>


#include "aeroswarm/sequential_simulation.hpp"

TEST_CASE("Simulation stores initial terrain and drones") {
    Terrain terrain{3, 3};

    std::vector<Drone> drones{
        Drone{1, {0, 0}},
        Drone{2, {2, 2}}
    };

    Simulation simulation{
        terrain,
        drones,
        42
    };

    REQUIRE(simulation.drones().size() == 2);
    REQUIRE(simulation.drones()[0].position() == Position{0, 0});
    REQUIRE(simulation.drones()[1].position() == Position{2, 2});
}


TEST_CASE("Simulation only one available neighbor") {
    Terrain terrain{3, 3};

    terrain.set_obstacle({0,1});
    terrain.set_obstacle({1,0});
    terrain.set_obstacle({1,2});

    std::vector<Drone> drones{
        Drone{1, {1, 1}}
    };

    Simulation simulation{
        terrain,
        drones,
        42
    };

    REQUIRE(simulation.step());
    REQUIRE(simulation.drones()[0].position() == Position{2, 1});
    REQUIRE(simulation.terrain().cell_at({2,1}).visited);
}


TEST_CASE("Simulation marks drone starting positions as visited") {
    Terrain terrain{3, 3};

    std::vector<Drone> drones{
        Drone{1, {0, 0}},
        Drone{2, {2, 2}}
    };

    Simulation simulation{
        terrain,
        drones,
        42
    };

    REQUIRE(simulation.terrain().cell_at({0, 0}).visited);
    REQUIRE(simulation.terrain().cell_at({2, 2}).visited);
}


TEST_CASE("Simulation detects when a drone reaches the target") {
    Terrain terrain{3,3};

    terrain.set_target({2,1});

    terrain.set_obstacle({0, 1});
    terrain.set_obstacle({1, 0});
    terrain.set_obstacle({1, 2});


    Simulation simulation{
        terrain,
        {Drone{7, {1,1}}},
        42
    };


    REQUIRE_FALSE(simulation.target_found());

    REQUIRE_FALSE(simulation.winning_drone_id().has_value());

    simulation.step();

    REQUIRE(simulation.winning_drone_id().has_value());
    REQUIRE(simulation.winning_drone_id().value() == 7);
    REQUIRE(simulation.drones()[0].position() == Position{2, 1});

}



TEST_CASE("Simulation runs until target is found") {
    Terrain terrain{3, 3};

    terrain.set_target({2, 1});
    terrain.set_obstacle({0, 1});
    terrain.set_obstacle({1, 0});
    terrain.set_obstacle({1, 2});

    Simulation simulation{
        terrain,
        {Drone{7, {1, 1}}},
        42
    };

    const auto status = simulation.run_until_done();

    REQUIRE(status == SimulationStatus::TargetFound);
    REQUIRE(simulation.target_found());
}


TEST_CASE("Simulation reports stuck when no drone can move") {
    Terrain terrain{3, 3};

    terrain.set_obstacle({0, 1});
    terrain.set_obstacle({2, 1});
    terrain.set_obstacle({1, 0});
    terrain.set_obstacle({1, 2});

    Simulation simulation{
        terrain,
        {Drone{1, {1, 1}}},
        42
    };

    const auto status = simulation.run_until_done();

    REQUIRE(status == SimulationStatus::Stuck);
    REQUIRE_FALSE(simulation.target_found());
}


TEST_CASE("Simulation is deterministic for the same seed") {
    Terrain terrain1{5,5};
    Terrain terrain2{5,5};

    terrain1.set_target({4, 4});
    terrain2.set_target({4, 4});

    std::vector<Drone> drones1{
        Drone{1, {0, 0}},
        Drone{2, {0, 4}}
    };

    std::vector<Drone> drones2{
        Drone{1, {0, 0}},
        Drone{2, {0, 4}}
    };

    Simulation sim1{terrain1, drones1, 42};
    Simulation sim2{terrain2, drones2, 42};

    const auto status1 = sim1.run_until_done();
    const auto status2 = sim2.run_until_done();
    
    
    REQUIRE(status1 == status2);
    REQUIRE(sim1.target_found() == sim2.target_found());
    REQUIRE(sim1.winning_drone_id() == sim2.winning_drone_id());

    REQUIRE(sim1.drones().size() == sim2.drones().size());

     for (std::size_t i = 0; i < sim1.drones().size(); ++i) {
        REQUIRE(
            sim1.drones()[i].position()
            == sim2.drones()[i].position()
        );
    }

}