
#include <catch2/catch_test_macros.hpp>
#include <string>

#include "aeroswarm/app/scenario_factory.hpp"



TEST_CASE("Scenario factory creates four corner drones") {
    const auto scenario =
        make_random_scenario(30, 30, 20, 42);

    REQUIRE(scenario.drones.size() == 4);

    REQUIRE(scenario.drones[0].position() == Position{0, 0});
    REQUIRE(scenario.drones[1].position() == Position{29, 0});
    REQUIRE(scenario.drones[2].position() == Position{0, 29});
    REQUIRE(scenario.drones[3].position() == Position{29, 29});
}


TEST_CASE("Scenario factory is deterministic for the same seed") {
    const auto a =
        make_random_scenario(30, 30, 20, 42);

    const auto b =
        make_random_scenario(30, 30, 20, 42);

    REQUIRE(a.target == b.target);
    REQUIRE(a.obstacles == b.obstacles);
}