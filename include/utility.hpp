#pragma once

#include <iostream> // For input/output operations
#include <string>   // For string operations
#include <vector>   // For using vectors
#include <algorithm> // For algorithms like std::sort
#include <filesystem> // For filesystem operations
#include <fstream> // For file operations
#include <random> // For random number generation
#include <unordered_map> // For unordered map operations
#include <unordered_set> // For unordered set operations, 
#include <chrono> // For time operations
#include <thread> // For multithreading operations 
#include <stdexcept> // For exception handling, like std::runtime_error
#include <functional> // For std::function, like : lambda functions
#include <memory> // For smart pointers





/// @brief  Enum class Cell to represent different types of cells in the grid
/// @details The Cell enum class defines the possible states of a cell in the grid.
/// It includes:
/// - Free: The cell is free and can be occupied by a drone.
/// - Obstacle: The cell is occupied by an obstacle and cannot be traversed.
/// - Target: The cell is a target for the drone to reach.
/// - Drone: The cell is occupied by a drone.
/// - Visited: The cell has been visited by the drone, used for path visualization.
enum class Cell {
    Free,
    Obstacle,
    Target,
    Drone, 
    Visited 
};


/// @brief Struct Position to represent a position in the grid
/// @details The Position struct contains two integer coordinates (x, y) to represent a position in a 2D grid.
/// It includes: [operator overleading] 
/// - Equality and inequality operators to compare positions.
/// - A less than operator to allow sorting or using Position in ordered containers.
/// @note The less than operator compares first by x coordinate, then by y coordinate.
/// This is useful for sorting or using Position in ordered containers like std::set or std::
struct Position {
    int x;
    int y;


    /// @brief  Equality and comparison operators for Position
    /// @param other 
    /// @return true if equal, false otherwise
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    /// @brief  Inequality and comparison operators for Position
    /// @param other 
    /// @return true if not equal, false otherwise
    bool operator!=(const Position& other) const {     
           return !(*this == other);  
    }

    /// @brief  Less than operator for Position
    /// @param other 
    /// @return true if this position is less than the other, false otherwise
    /// This is useful for sorting or using Position in ordered containers
    /// like std::set or std::map.
    /// It compares first by x coordinate, then by y coordinate.
    bool operator<(const Position& other) const {     
           return (x < other.x) || (x == other.x && y < other.y);
    }   

};


/*
- A hash is like a label that tells you which box a Position goes into.
- Without a label, you’d have to look into every box (slow).
- With a hash label, you jump straight to the right box (fast!).

C++ provides a generic hash template inside namespace std

namespace std {
    template<class T> struct hash; // generic hash
}

namespace std { }, the compiler wouldn’t recognize 
your hash as the one the standard containers need

*/
/// @brief  Hash function for Position
/// @details This specialization of std::hash allows Position to be used as a key in unordered
/// containers like std::unordered_map or std::unordered_set.
/// It combines the hashes of the x and y coordinates using XOR to produce a unique hash value
/// for each Position.
/// @note This is necessary because Position is a user-defined type and does not have a default
/// hash function provided by the standard library.
namespace std {
    template <>
    struct hash<Position> {
        size_t operator()(const Position& pos) const {
            // XOR : a logical operation that outputs true (1) if 
            // and only if one of its inputs is true (1), but not both
            //return hash<int>()(pos.x) ^ hash<int>()(pos.y); // collisions if (x, y) and (y, x) produce the same hash.
            return hash<int>()(pos.x) ^ (hash<int>()(pos.y) << 1); // Shift y hash to reduce collisions
        }
    };
} // namespace std




void create_frames_directory(const std::string& path);

