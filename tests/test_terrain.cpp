#include <catch2/catch_test_macros.hpp>
#include "aeroswarm/terrain.hpp"

TEST_CASE("Terrain reports valid bounds") {
    Terrain terrain{3, 4};

    REQUIRE(terrain.in_bounds({0, 0}));
    REQUIRE(terrain.in_bounds({2, 3}));

    REQUIRE_FALSE(terrain.in_bounds({-1, 0}));
    REQUIRE_FALSE(terrain.in_bounds({0, -1}));
    REQUIRE_FALSE(terrain.in_bounds({3, 0}));
    REQUIRE_FALSE(terrain.in_bounds({0, 4}));
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
