#include <catch2/catch_test_macros.hpp>  // Modern include
#include "drone.hpp"
#include "terrain.hpp"
#include "utility.hpp"


TEST_CASE("Drone Initialization", "[drone][basic]") {
    // Setup
    const int width = 10;
    const int height = 10;
    Terrain terrain(width, height);
    Position start_pos{2, 3};
    int drone_id = 0;

    // Test
    Drone drone(drone_id, start_pos, terrain);

   

    SECTION("Terrain is marked correctly") {
        REQUIRE(terrain.grid[start_pos.x][start_pos.y] == Cell::Drone);
    }

    SECTION("ID is assigned correctly") {
        REQUIRE(drone.get_id() == drone_id);
    }
}

TEST_CASE("Drone Boundary Conditions", "[drone][edge]") {
    Terrain terrain(10, 10);
    
    SECTION("Invalid position throws") {
        REQUIRE_THROWS_AS(Drone(0, {-1, 5}, terrain), std::runtime_error);
    }
}

TEST_CASE("Drone Movement", "[drone][movement]") {
    Terrain terrain(10, 10);
    Drone drone(0, {1, 1}, terrain);
    
    SECTION("Valid move updates position") {
        Position new_pos{1, 2};
        drone.move_to(new_pos);
        REQUIRE(terrain.grid[new_pos.x][new_pos.y] == Cell::Drone);
    }
}