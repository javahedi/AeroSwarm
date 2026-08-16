#include <catch2/catch_test_macros.hpp>  // Modern include
#include "drone.hpp"
#include "terrain.hpp"
#include "utility.hpp"


TEST_CASE("Drone Initialization") {
    Terrain terrain(10, 10);
    Position pos{2, 3};
    Drone drone(0, pos, terrain);

    SECTION("Terrain marked correctly") {
        REQUIRE(terrain.get_cell(pos.x, pos.y) == Cell::Drone);
    }
}

TEST_CASE("Drone Movement") {
    Terrain terrain(10, 10);
    Position start{1, 1};
    Drone drone(0, start, terrain);
    Position end{1, 2};

    SECTION("Valid move updates terrain") {
        drone.move_to(end);
        REQUIRE(terrain.get_cell(end.x, end.y) == Cell::Drone);
        REQUIRE(terrain.get_cell(start.x, start.y) == Cell::Visited);
    }
}
// use command  " ctest" or "./bin/tests"