#pragma once

#include <vector>
#include <array> // zero heap allocation
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <optional>
#include "aeroswarm/types.hpp"

/*
Fixed-capacity container for neighboring terrain positions.

Why not std::vector<Position>?

A drone can have at most 8 neighbors:

    NW   N   NE
      \  |  /
    W -- D -- E
      /  |  \
    SW   S   SE

The maximum capacity is therefore known at compile time.

The previous implementation used std::vector<Position>, which may
dynamically allocate and grow its storage while available_neighbors()
is running. Since neighbor discovery is part of the simulation hot path,
we instead use std::array<Position, 8>.

std::array:
    - has fixed capacity
    - stores its elements directly inside the object
    - does not dynamically allocate storage for its elements
    - provides contiguous storage
    - supports STL-style iterators

However, std::array<Position, 8> always contains 8 Position objects,
while a drone may currently have fewer than 8 valid neighbors.

Therefore:

    positions.size() == 8       // physical capacity
    count                       // logical number of valid neighbors

Example:

    positions:
    +-----+-----+-----+-----+-----+-----+-----+-----+
    | P0  | P1  | P2  |  -  |  -  |  -  |  -  |  -  |
    +-----+-----+-----+-----+-----+-----+-----+-----+

    count = 3

Only P0, P1 and P2 are logically part of this Neighbors collection.

The helper functions below intentionally give Neighbors a small
STL-container-like interface so existing simulation code can use:

    neighbors.empty()
    neighbors.size()
    neighbors[i]

and:

    for (const auto& neighbor : neighbors)

without knowing that the underlying storage is std::array.
*/
struct Neighbors {
    // Fixed storage for the maximum possible number of neighbors.
    // No vector growth or dynamic element-storage allocation is required.
    std::array<Position, 8> positions{};

    // Number of positions currently containing valid neighbors.
    // This is the logical size of the container, not its capacity.
    std::size_t count{0};


    /*
    Allows:

        if (neighbors.empty()) {
            ...
        }

    We cannot use positions.empty() for this purpose because
    std::array<Position, 8> is never empty: its size is always 8.

    Instead, our logical container is empty when count == 0.
    */
    bool empty() const {
        return count == 0;
    }


    /*
    Allows:

        neighbors.size()

    positions.size() always returns 8 because that is the physical
    capacity of the std::array.

    Our size() returns the number of VALID neighbors.
    */
    std::size_t size() const {
        return count;
    }


    /*
    Non-const begin().

    Allows iteration over a mutable Neighbors object.

    Example:

        for (auto& neighbor : neighbors) {
            ...
        }

    positions.begin() points to the first element of the array.
    */
    auto begin() {
        return positions.begin();
    }


    /*
    Non-const end().

    IMPORTANT:
    positions.end() would point after all 8 array elements.

    But perhaps only the first 3 positions are valid.

    Therefore our logical end is:

        positions.begin() + count

    Example with count == 3:

        begin()
          |
          v
        [P0][P1][P2][--][--][--][--][--]
                    ^
                    |
                   end()

    This is what makes range-based for loops visit only valid neighbors.
    */
    auto end() {
        return positions.begin() + count;
    }


    /*
    Const begin().

    Used when the Neighbors object itself is const.

    Example:

        const auto neighbors =
            terrain.available_neighbors(position);

        for (const auto& neighbor : neighbors) {
            ...
        }

    Because neighbors is const, C++ needs const-compatible
    begin()/end() functions.
    */
    auto begin() const {
        return positions.begin();
    }


    /*
    Const version of end().

    Again, the logical end is determined by count rather than the
    physical end of the 8-element std::array.
    */
    auto end() const {
        return positions.begin() + count;
    }


    /*
    Const indexing operator.

    Allows:

        const Neighbors neighbors = ...;

        const Position& p = neighbors[2];

    Returning const Position&:
        - avoids copying Position
        - prevents modification through a const Neighbors object
    */
    const Position& operator[](std::size_t index) const {
        return positions[index];
    }


    /*
    Non-const indexing operator.

    Allows:

        Neighbors neighbors;
        neighbors[0] = Position{1, 2};

    Returning Position& gives direct mutable access to the stored
    Position object.
    */
    Position& operator[](std::size_t index) {
        return positions[index];
    }
};






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

    /*
    BEFORE

    std::vector<Position>
            │
            ├── dynamic capacity
            ├── allocator involvement
            └── 1031 growth events / 274 calls


    AFTER

    Neighbors
    ┌──────────────────────┐
    │ std::array<Pos, 8>   │
    │ count                │
    └──────────────────────┘
            │
            ├── fixed capacity
            ├── storage embedded in object
            └── 0 vector growth events

    */
    std::vector<Position> available_neighbors_vector(const Position& pos) const {
        std::lock_guard<std::mutex> lock(mtx_);

        validate_position(pos);

        neighbor_calls_.fetch_add(1);

        std::vector<Position> candidates;

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

            if (candidates.capacity() !=previous_capacity) {
                neighbor_capacity_growths_.fetch_add(1);
                previous_capacity = candidates.capacity();
            }
        }

        return candidates;
    }

    std::size_t neighbor_calls() const {
        return neighbor_calls_.load();
    }

    std::size_t neighbor_capacity_growths() const {
        return neighbor_capacity_growths_.load();
    }

    
    Neighbors available_neighbors(const Position& pos) const {
        std::lock_guard<std::mutex> lock(mtx_);

        validate_position(pos);

        Neighbors candidates;

        for (const auto& dir : directions_) {
            const Position next = pos + dir;

            if (!in_bounds(next)) {
                continue;
            }

            if (grid_[next.x][next.y].type ==
                CellType::Obstacle) {
                continue;
            }

            if (grid_[next.x][next.y].visited) {
                continue;
            }

            if (candidates.count >= candidates.positions.size()) {
                throw std::logic_error(
                    "Neighbor capacity exceeded"
                );
            }

            candidates.positions[candidates.count] = next;
            ++candidates.count;
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

    // mutabl, ince if a finction is "const" >>
    // ParallelTerrain::available_neighbors(const Position& pos) const;
    // wihtout mutable>> the function won't let changing neighbor_calls_ and neighbor_capacity_growths_
    mutable std::atomic<std::size_t> neighbor_calls_{0};
    mutable std::atomic<std::size_t> neighbor_capacity_growths_{0};
    // why atomic>> since 
    //Worker 0 ─┐
    //Worker 1 ─┤
    //Worker 2 ─┼──→ available_neighbors()
    //Worker 3 ─┘
    // ++neighbor_calls_; // ❌ multiple threads
    // then atomic>> do everthiogn we need to prevent DataRace ;-)
    // 
};