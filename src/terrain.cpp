#include "terrain.hpp"
#include <fstream>
#include <random>
#include <stdexcept>
#include <cassert>


Terrain::Terrain(int w, int h) : width(w), height(h), grid(w, std::vector<Cell>(h, Cell::Free)) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dis(0.1); // 10% chance for obstacles

    // Place obstacles (10% probability)
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (dis(gen)) {
                grid[x][y] = Cell::Obstacle;
            }
        }
    }

    // Place target at random free position
    while (true) {
        int x = std::uniform_int_distribution<>(0, width - 1)(gen);
        int y = std::uniform_int_distribution<>(0, height - 1)(gen);
        if (grid[x][y] == Cell::Free) {
            target = {x, y};
            grid[x][y] = Cell::Target;
            break;
        }
    }
}


bool Terrain::is_valid_position(const Position& pos) const {
    //std::lock_guard<std::mutex> lock(terrain_mutex);
    std::shared_lock<std::shared_mutex> lock(terrain_mutex); // Many drones can check positions in parallel
    //std::unique_lock<std::shared_mutex> lock(terrain_mutex); // Only one drone can modify terrain,,  Exclusive write

    
    // DEBUG:
    //std::cout << "Checking position (" << pos.x << "," << pos.y << ") - ";
    bool valid = pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height &&
               (grid[pos.x][pos.y] == Cell::Free || 
                grid[pos.x][pos.y] == Cell::Target || 
                grid[pos.x][pos.y] == Cell::Visited);
    
    // DEBUG:
    // std::cout << (valid ? "VALID" : "INVALID") << "\n";
    
    return valid;
}


void Terrain::place_drone(const Position& pos) {
    // First validate with shared lock
    {
        std::shared_lock<std::shared_mutex> read_lock(terrain_mutex);
        if (!is_valid_position(pos)) {
            throw std::runtime_error("Invalid position for drone placement");
        }
    }

    // Then modify with exclusive lock
    std::unique_lock<std::shared_mutex> write_lock(terrain_mutex);
    grid[pos.x][pos.y] = Cell::Drone;
    drone_positions.push_back(pos);
}

void Terrain::clear_drone_position(const Position& pos) {
    //std::lock_guard<std::mutex> lock(terrain_mutex);
    std::unique_lock<std::shared_mutex> lock(terrain_mutex);

    // First check if position is within bounds
    if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) {
        debug_invalid_position(pos, "Position out of bounds");
        throw std::runtime_error("Position out of bounds");
    }

    // Then check if it's an obstacle
    if (grid[pos.x][pos.y] == Cell::Obstacle) {
        debug_invalid_position(pos, "Cannot clear obstacle cell");
        throw std::runtime_error("Cannot clear obstacle cell");
    }

    // Check if position actually contains a drone
    auto it = std::find(drone_positions.begin(), drone_positions.end(), pos);

    if (it == drone_positions.end()) {
        debug_invalid_position(pos, "No drone at specified position");
        throw std::runtime_error("No drone at specified position");
    }

    // Clear the position (preserve targets)
    grid[pos.x][pos.y] = (pos == target) ? Cell::Target : Cell::Visited;
    
    // Remove from drone positions 
    drone_positions.erase(it);
        
}


// Helper debug function
void Terrain::debug_invalid_position(const Position& pos, const std::string& message) const {
    std::cerr << "ERROR: "        << message << " at (" << pos.x << "," << pos.y << ")\n";
    std::cerr << "Terrain size: " << width << "x" << height << "\n";
    std::cerr << "Target position: (" << target.x << "," << target.y << ")\n";
    std::cerr << "Current cell type: ";
    
    switch(grid[pos.x][pos.y]) {
        case Cell::Free: std::cerr << "Free"; break;
        case Cell::Obstacle: std::cerr << "Obstacle"; break;
        case Cell::Target: std::cerr << "Target"; break;
        case Cell::Drone: std::cerr << "Drone"; break;
        case Cell::Visited: std::cerr << "Visited"; break;
    }
    std::cerr << "\nActive drones: " << drone_positions.size() << std::endl;
}


bool Terrain::save_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return false;
    }

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            switch (grid[x][y]) {
                case Cell::Free: file       << "⬜"; break;
                case Cell::Obstacle: file   << "⬛"; break;
                case Cell::Target: file     << "🎯"; break;
                case Cell::Drone: file      << "🛸"; break;
                case Cell::Visited: file    << "X"; break;
            }
        }
        file << "\n";
    }
    return true;
}