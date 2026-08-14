#pragma once

#include <vector>
#include "aeroswarm/types.hpp"

class Terrain {
public:
    Terrain(int w, int h)
        : width_(w),
          height_(h),
          grid_(w, std::vector<Cell>(h))
    {
    }

    bool in_bounds(const Position& pos) const {
        return pos.x >= 0 &&
               pos.x < width_ &&
               pos.y >= 0 &&
               pos.y < height_;
    }

    const Cell& cell_at(const Position& pos) const {
        return grid_[pos.x][pos.y];
    }

    void mark_visited(const Position& pos) {
        grid_[pos.x][pos.y].visited = true;
    }

    void set_obstacle(const Position& pos) {
        grid_[pos.x][pos.y].type = CellType::Obstacle;
    }

    void set_target(const Position& pos) {
        grid_[pos.x][pos.y].type = CellType::Target;
    }

private:
    int width_;
    int height_;
    std::vector<std::vector<Cell>> grid_;
};