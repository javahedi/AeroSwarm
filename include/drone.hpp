#pragma once

#include "utility.hpp"
#include "terrain.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

// Drone class
/// @brief Represents a drone navigating through a terrain.
/// @details The Drone class manages the drone's position, path, and exploration strategy.
/// It tracks visited positions, counts visits, and uses heuristics (least visited, highest
/// information gain) to choose the next move. The drone aims to find a target without prior
/// knowledge of its location, avoiding obstacles and exploring efficiently.
class Drone {
public:
    Position position;                                  // Current position of the drone
    Terrain& terrain;                                   // Reference to the terrain the drone is navigating
    std::unordered_map<Position, int> visit_count;      // Count of visits to each position
    std::unordered_set<Position> visited_positions;     // Set of visited positions
    std::vector<Position> neighbors;                    // List of neighboring positions
    bool target_found;                                  // Flag to indicate if the target has been found

    /// @brief Constructor to initialize the drone with a position and terrain reference.
    /// @param pos Initial position of the drone.
    /// @param t Reference to the terrain the drone is navigating.
    Drone(const Position& pos, Terrain& t);

    /// @brief Move the drone to a new position.
    /// @details Updates the drone's position, increments visit count, and marks the new position
    /// as visited. Sets target_found if the new position is the target.
    /// @param new_pos New position to move the drone to.
    void move_to(const Position& new_pos);

    /// @brief Retrieve valid neighboring positions.
    /// @details Returns adjacent positions (up, down, left, right, diagonals) that are within
    /// bounds and not obstacles. Returns only the target if it’s adjacent.
    /// @return A vector of valid neighboring positions.
    std::vector<Position> get_neighbors();

    /// @brief Calculate information gain for a position.
    /// @details Counts the number of valid neighboring positions (free, target, or visited)
    /// around the given position.
    /// @param pos Position to evaluate.
    /// @return Number of valid neighbors.
    int get_information_gain(const Position& pos) const;

    /// @brief Select neighbors with the least visit count.
    /// @details From the given candidates, returns those with the lowest visit count.
    /// @param candidates List of positions to evaluate.
    /// @return Vector of least-visited positions.
    std::vector<Position> get_least_visited_neighbors(const std::vector<Position>& candidates) const;

    /// @brief Select neighbors with the highest information gain.
    /// @details From the given candidates, returns those with the most valid neighbors.
    /// @param candidates List of positions to evaluate.
    /// @return Vector of highest-gain positions.
    std::vector<Position> get_highest_info_gain_neighbors(const std::vector<Position>& candidates) const;

    /// @brief Determine the next position to move to.
    /// @details Uses heuristics: selects least-visited neighbors, then those with highest
    /// information gain, and randomly chooses among them.
    /// @return The next position to move to.
    Position find_next_position();

    /// @brief Check if the drone has reached the target.
    /// @return True if the drone is at the target position, false otherwise.
    bool has_reached_target() const;

    /// @brief Save the current terrain state to a file.
    /// @param filename Name of the file to save to.
    /// @return True if saved successfully, false otherwise.
    bool save_terrain_to_file(const std::string& filename) const;
};