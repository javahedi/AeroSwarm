#pragma once

#include <vector>
#include <mutex>
#include <stdexcept>
#include <optional>
#include "aeroswarm/types.hpp"


class ParallelTerrain {
public:
    ParallelTerrain(int w, int h)
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


    bool try_claim_cell(const Position& pos) {
           
        std::lock_guard<std::mutex> lock(mtx_);

        if (!in_bounds(pos)) {
            return false;
        }
        if (grid_[pos.x][pos.y].type == CellType::Obstacle) 
        {
            return false;
        } 

        if (grid_[pos.x][pos.y].visited) 
        {
            return false;
        } 
        grid_[pos.x][pos.y].visited = true;
        return true;

    }


    std::vector<Position> available_neighbors(const Position& pos) const {
        std::lock_guard<std::mutex> lock(mtx_);

        validate_position(pos);

        std::vector<Position> candidates;

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
        }

        return candidates;
    }


    void set_obstacle(const Position& pos) {
        std::lock_guard<std::mutex> lock(mtx_);
        validate_position(pos);
        grid_[pos.x][pos.y].type = CellType::Obstacle;
    }


    void set_target(const Position& pos) {
        std::lock_guard<std::mutex> lock(mtx_);
        validate_position(pos);
        grid_[pos.x][pos.y].type = CellType::Target;
    }

    bool is_target(const Position& pos) const {
        std::lock_guard<std::mutex> lock(mtx_);

        validate_position(pos);
        return grid_[pos.x][pos.y].type == CellType::Target;
    }


    bool initialize_start_position(const Position& pos) {
        std::lock_guard<std::mutex> lock(mtx_);

        if (!in_bounds(pos)) {
            return false;
        }

        if (grid_[pos.x][pos.y].type == CellType::Obstacle) {
            return false;
        }

        if (grid_[pos.x][pos.y].visited) {
            return true;
        }

        grid_[pos.x][pos.y].visited = true;
        return true;
    }



    std::vector<Position> visited_positions() const {
        std::lock_guard<std::mutex> lock(mtx_);

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
        std::lock_guard<std::mutex> lock(mtx_);

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
        std::lock_guard<std::mutex> lock(mtx_);

        for (int x = 0; x < width_; ++x) {
            for (int y = 0; y < height_; ++y) {
                if (grid_[x][y].type == CellType::Target) {
                    return Position{x, y};
                }
            }
        }

        return std::nullopt;
    }

    int information_gain(const Position& pos) const {
        std::lock_guard<std::mutex> lock(mtx_);

        if (!in_bounds(pos)) {
            return 0;
        }

        int gain = 0;

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

            ++gain;
        }

        return gain;
}

private:
    int width_;
    int height_;

    std::vector<std::vector<Cell>> grid_;
    
    // why mutable>> some methods are "cosnt" so if mtx_ not be mutable>> 
    // they can not lock/unlock it insided themselef>> such as  is_target
    mutable std::mutex mtx_; 
    


    const std::vector<Position> directions_{
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };
    

    void validate_position(const Position& pos) const {
        if (!in_bounds(pos)) {
            throw std::out_of_range("Position is outside terrain bounds");
        }


    }
};