#include <catch2/catch_test_macros.hpp>
#include "aeroswarm/drone.hpp"


TEST_CASE("initialization") {
    Drone drone{1, {1,1}};

    REQUIRE(drone.position() == Position{1, 1});
}

TEST_CASE(" move_to()") {
    Drone drone{1, {1,1}};
    drone.move_to({1,2});
    REQUIRE(drone.position() == Position{1, 2});
}

