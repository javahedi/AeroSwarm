#include <catch2/catch_test_macros.hpp>

#include <string>

#include "aeroswarm/app/scenario.hpp"
#include "aeroswarm/app/scenario_validation.hpp"


TEST_CASE("Scenario validation accepts a valid scenario") {
    Scenario scenario;

    std::string error;

    REQUIRE(validate_scenario(scenario, error));
    REQUIRE(error.empty());
}


TEST_CASE("Scenario validation rejects non-positive dimensions") {
    Scenario scenario;
    scenario.width = 0;

    std::string error;

    REQUIRE_FALSE(validate_scenario(scenario, error));
    REQUIRE_FALSE(error.empty());
}


TEST_CASE("Scenario validation rejects target outside terrain") {
    Scenario scenario;
    scenario.target = {100, 100};

    std::string error;

    REQUIRE_FALSE(validate_scenario(scenario, error));
    REQUIRE_FALSE(error.empty());
}


TEST_CASE("Scenario validation rejects obstacle outside terrain") {
    Scenario scenario;
    scenario.obstacles.push_back({100, 100});

    std::string error;

    REQUIRE_FALSE(validate_scenario(scenario, error));
    REQUIRE_FALSE(error.empty());
}


TEST_CASE("Scenario validation rejects obstacle on target") {
    Scenario scenario;
    scenario.obstacles.push_back(scenario.target);

    std::string error;

    REQUIRE_FALSE(validate_scenario(scenario, error));
    REQUIRE_FALSE(error.empty());
}


TEST_CASE("Scenario validation rejects drone starting on obstacle") {
    Scenario scenario;

    const Position start = scenario.drones.front().position();

    scenario.obstacles.push_back(start);

    std::string error;

    REQUIRE_FALSE(validate_scenario(scenario, error));
    REQUIRE_FALSE(error.empty());
}


TEST_CASE("Scenario validation allows drones to share start position") {
    Scenario scenario;

    scenario.drones = {
        Drone{1, {1, 1}},
        Drone{2, {1, 1}}
    };

    std::string error;

    REQUIRE(validate_scenario(scenario, error));
    REQUIRE(error.empty());
}