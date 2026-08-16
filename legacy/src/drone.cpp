#include "drone.hpp"
#include <random>
#include <limits>
#include <stdexcept>

Drone::Drone(int id, const Position& pos, Terrain& t) 
    : position(pos), terrain(t), target_found(false), id_(id) {
    visit_count[pos] = 1;
    visited_positions.insert(pos);
    terrain.place_drone(position);
}



void Drone::move_to(const Position& new_pos) {
    if (!terrain.is_valid_position(new_pos)) {
        throw std::runtime_error("Invalid move position");
    }

    // Clear old position
    terrain.clear_drone_position(position);
    
    // Update position
    position = new_pos;
    
    // Mark new position
    terrain.place_drone(position);

    // Update visit tracking
    visit_count[new_pos]++;
    visited_positions.insert(new_pos);

    if (new_pos == terrain.target) {
        target_found = true;
    }
}

std::vector<Position> Drone::get_neighbors() {

    std::vector<Position> neighbors;

    std::vector<Position> directions = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}, // Up, Down, Right, Left
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // Diagonals
    };

    for (const auto& dir : directions) {
        Position neighbor = {position.x + dir.x, position.y + dir.y};
        if (terrain.is_valid_position(neighbor)) {
            neighbors.push_back(neighbor);
            if (terrain.grid[neighbor.x][neighbor.y] == Cell::Target) {
                return {neighbor}; // Move directly to target if adjacent
            }
        }
    }
    return neighbors;
}

int Drone::get_information_gain(const Position& pos) const {
    int gain = 0;
    std::vector<Position> directions = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (const auto& dir : directions) {
        Position neighbor = {pos.x + dir.x, pos.y + dir.y};
        if (terrain.is_valid_position(neighbor)) {
            gain++;
        }
    }
    return gain;
}

std::vector<Position> Drone::get_least_visited_neighbors(const std::vector<Position>& candidates) const {
    if (candidates.empty()) {
        return {};
    }

    std::vector<Position> least_visited;
    int min_visits = std::numeric_limits<int>::max(); // ? Initialize to max int

    for (const auto& neighbor : candidates) {
        int visits = visit_count.find(neighbor) != visit_count.end() ? visit_count.at(neighbor) : 0;
        if (visits < min_visits) {
            min_visits = visits;
            least_visited = {neighbor};
        } else if (visits == min_visits) {
            least_visited.push_back(neighbor);
        }
    }

    return least_visited;
}

std::vector<Position> Drone::get_highest_info_gain_neighbors(const std::vector<Position>& candidates) const {
    if (candidates.empty()) {
        return {};
    }

    std::vector<Position> highest_gain;
    int max_gain = -1;

    for (const auto& neighbor : candidates) {
        int gain = get_information_gain(neighbor);
        if (gain > max_gain) {
            max_gain = gain;
            highest_gain = {neighbor};
        } else if (gain == max_gain) {
            highest_gain.push_back(neighbor);
        }
    }

    return highest_gain;
}

Position Drone::find_next_position() {
    auto neighbors = get_neighbors();
    
    // Prioritize target if visible
    for (const auto& neighbor : neighbors) {
        if (terrain.grid[neighbor.x][neighbor.y] == Cell::Target) {
            return neighbor;
        }
    }

    //std::vector<Position> least_visited   = get_least_visited_neighbors(neighbors);
    //std::vector<Position> best_candidates = get_highest_info_gain_neighbors(least_visited);

    // Filter by least visited and information gain
    auto least_visited = get_least_visited_neighbors(neighbors);
    auto best_candidates = get_highest_info_gain_neighbors(least_visited);
    
     if (best_candidates.empty()) {
        throw std::runtime_error("No valid moves");
    }

    // Randomly select from the best candidates
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, best_candidates.size() - 1);
    return best_candidates[dis(gen)];
}

bool Drone::has_reached_target() const {
    return position == terrain.target;
}

bool Drone::save_terrain_to_file(const std::string& filename) const {
    return terrain.save_to_file(filename);
}

int Drone::get_id() const {
    return id_;
}