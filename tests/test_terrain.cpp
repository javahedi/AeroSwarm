#include <catch2/catch_test_macros.hpp>
#include "aeroswarm/sequential/terrain.hpp"

#include <stdexcept>
#include <algorithm>

TEST_CASE("Terrain reports valid bounds") {
    Terrain terrain{3, 4};

    REQUIRE(terrain.in_bounds({0, 0}));
    REQUIRE(terrain.in_bounds({2, 3}));

    REQUIRE_FALSE(terrain.in_bounds({-1, 0}));
    REQUIRE_FALSE(terrain.in_bounds({0, -1}));
    REQUIRE_FALSE(terrain.in_bounds({3, 0}));
    REQUIRE_FALSE(terrain.in_bounds({0, 4}));
}


TEST_CASE("Terrain rejects out-of-bounds cell access") {
    Terrain terrain{3, 4};

    REQUIRE_THROWS_AS(
        terrain.cell_at({3, 0}),
        std::out_of_range
    );

    REQUIRE_THROWS_AS(
        terrain.cell_at({-1, 0}),
        std::out_of_range
    );
}

TEST_CASE("Terrain cells are free and unvisited by default") {
    Terrain terrain{3, 4};

    const Cell& cell = terrain.cell_at({1, 2});

    REQUIRE(cell.type == CellType::Free);
    REQUIRE_FALSE(cell.visited);
}

TEST_CASE("Terrain can mark a cell as visited") {
    Terrain terrain{3, 4};

    terrain.mark_visited({1, 2});

    REQUIRE(terrain.cell_at({1, 2}).visited);
}

TEST_CASE("Terrain can place obstacles") {
    Terrain terrain{3, 4};

    terrain.set_obstacle({1, 2});

    REQUIRE(terrain.cell_at({1, 2}).type == CellType::Obstacle);
}

TEST_CASE("Terrain can place a target") {
    Terrain terrain{3, 4};

    terrain.set_target({1, 2});

    REQUIRE(terrain.cell_at({1, 2}).type == CellType::Target);
}


TEST_CASE("Terrain returns available unvisited neighbors") {
    Terrain terrain{3, 3};

    REQUIRE(terrain.available_neighbors({1,1}).size() == 4);
    terrain.mark_visited({2,1});
    REQUIRE(terrain.available_neighbors({1,1}).size() == 3);
    terrain.set_obstacle({0,1});
    REQUIRE(terrain.available_neighbors({1,1}).size() == 2);

}


TEST_CASE("Terrain returns the correct neighbors") {
    Terrain terrain{3, 3};

    terrain.mark_visited({2, 1});
    terrain.set_obstacle({0, 1});

    const auto neighbors = terrain.available_neighbors({1, 1});

    REQUIRE(neighbors.size() == 2);

    REQUIRE(std::find(neighbors.begin(), neighbors.end(), Position{1, 0}) != neighbors.end());
    REQUIRE(std::find(neighbors.begin(), neighbors.end(), Position{1, 2}) != neighbors.end());
}

TEST_CASE("Terrain excludes out-of-bounds neighbors") {
    Terrain terrain{3, 3};

    const auto neighbors = terrain.available_neighbors({0, 0});

    REQUIRE(neighbors.size() == 2);
    REQUIRE(std::find(neighbors.begin(), neighbors.end(), Position{1, 0}) != neighbors.end());
    REQUIRE(std::find(neighbors.begin(), neighbors.end(), Position{0, 1}) != neighbors.end());
}