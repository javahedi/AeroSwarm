#pragma once

#include <vector>
#include "aeroswarm/types.hpp"
#include <stdexcept>


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
        validate_position(pos);
        return grid_[pos.x][pos.y];
    }

    void mark_visited(const Position& pos) {
        validate_position(pos);
        grid_[pos.x][pos.y].visited = true;
    }

    void set_obstacle(const Position& pos) {
        validate_position(pos);
        grid_[pos.x][pos.y].type = CellType::Obstacle;
    }

    void set_target(const Position& pos) {
        validate_position(pos);
        grid_[pos.x][pos.y].type = CellType::Target;
    }

    std::vector<Position> available_neighbors(const Position& pos) const {
        validate_position(pos);

        std::vector<Position> candidates;

        for (const auto& dir : directions_) {
            Position next = pos + dir;

            if (!in_bounds(next)) {
                continue;
            }

            if (grid_[next.x][next.y].type == CellType::Obstacle) {
                continue;
            }

            if (grid_[next.x][next.y].visited) {
                continue;
            }

            candidates.push_back(next);
        }

        return candidates;
    }

private:
    int width_;
    int height_;
    std::vector<std::vector<Cell>> grid_;
    
    const std::vector<Position> directions_{
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };
    

    void validate_position(const Position& pos) const {
        if (!in_bounds(pos)) {
            throw std::out_of_range("Position is outside terrain bounds");
        }
    }
};