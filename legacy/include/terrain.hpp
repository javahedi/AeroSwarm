#pragma once

#include <mutex>
#include <shared_mutex>
#include "utility.hpp"

// @brief  Terrain class to represent the grid terrain
/// @details The Terrain class contains a 2D grid of cells, a target position,
/// and a list of drone paths.
/// It provides methods to initialize the terrain, place obstacles, and set a target.
/// The grid is represented as a vector of vectors of Cell enum values.
/// The target is a Position object representing the target cell in the grid.
/// The drone_paths vector stores the paths of drones as Position objects. 
class Terrain {
public:
    int width;
    int height;                         // Dimensions of the terrain grid
    Position target;                      // Position of the target cell
    std::vector<std::vector<Cell>> grid; // 2D grid representing the terrain
    std::vector<Position> drone_positions;  // Stores the positions of drones

     // Mutex for thread-safe access to the terrain
    // why mutable? Because we need to modify the mutex even in const "is_valid_position" member functions.
    //mutable std::mutex terrain_mutex;      // Locks ALL access (reads/writes)
    mutable std::shared_mutex terrain_mutex; // Allows parallel reads
      


    /// @brief Constructor to initialize the terrain with given width and height
    /// @details The constructor initializes the grid with the specified width and height,
    /// fills it with free cells, places obstacles randomly, and sets a target position.
    /// @param w Width of the terrain grid
    /// @param h Height of the terrain grid
    /// @note The constructor uses a random number generator to place obstacles with a 10%
    Terrain(int w, int h);
    bool is_valid_position(const Position& pos) const;
    void place_drone(const Position& pos);
    void clear_drone_position(const Position& pos);
    void debug_invalid_position(const Position& pos, const std::string& message) const;
    bool save_to_file(const std::string& filename) const;

    Cell get_cell(int x, int y) const; 
};
