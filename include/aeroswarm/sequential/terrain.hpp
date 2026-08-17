#pragma once

#include <vector>
#include "aeroswarm/types.hpp"
#include <stdexcept>
#include <optional>


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

    /*
    checking heap allocation of std::vector
    
    std::vector<Position> available_neighbors(const Position& pos) const {
        std::lock_guard<std::mutex> lock(mtx_);

        validate_position(pos);

        std::vector<Position> candidates;

        std::size_t allocation_events = 0;
        std::size_t previous_capacity = candidates.capacity();

        for (const auto& dir : directions_) {
            const Position next = pos + dir;

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

            if (candidates.capacity() != previous_capacity) {
                ++allocation_events;
                previous_capacity = candidates.capacity();
            }
        }

        if (allocation_events > 0) {
            std::cout
                << "[neighbors] size=" << candidates.size()
                << " capacity=" << candidates.capacity()
                << " growth_events=" << allocation_events
                << '\n';
        }

        return candidates;
    }
    
    */


    std::vector<Position> visited_positions() const {
        std::vector<Position> positions;

        for (int x = 0; x < width_; ++x) {
            for (int y = 0; y < height_; ++y) {
                if (grid_[x][y].visited) {
                    positions.push_back({x, y});
                }
            }
        }

        return positions;
    }


    std::vector<Position> obstacle_positions() const {
        std::vector<Position> positions;

        for (int x = 0; x < width_; ++x) {
            for (int y = 0; y < height_; ++y) {
                if (grid_[x][y].type == CellType::Obstacle) {
                    positions.push_back({x, y});
                }
            }
        }

        return positions;
    }


    std::optional<Position> target_position() const {
        for (int x = 0; x < width_; ++x) {
            for (int y = 0; y < height_; ++y) {
                if (grid_[x][y].type == CellType::Target) {
                    return Position{x, y};
                }
            }
        }

        return std::nullopt;
    }

private:
    int width_;
    int height_;
    std::vector<std::vector<Cell>> grid_;
    
    const std::vector<Position> directions_{
        {1,0}, {-1,0}, {0,1}, {0,-1}, 
        //{1,1}, {-1,-1}, {-1,1}, {1,-1},
    };
    

    void validate_position(const Position& pos) const {
        if (!in_bounds(pos)) {
            throw std::out_of_range("Position is outside terrain bounds");
        }
    }


    
};