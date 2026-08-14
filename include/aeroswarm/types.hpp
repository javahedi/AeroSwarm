#pragma once

enum class CellType {
    Free,
    Obstacle,
    Target
};

struct Position {
    int x{0};
    int y{0};
    /*
    bool operator==(const Position& other) const
                ^^^^^^^^^^^^^^^^^^^^^  ^^^^^
                don't modify other     don't modify *this
    */
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Cell {
    CellType type{CellType::Free};
    bool visited{false};
};
