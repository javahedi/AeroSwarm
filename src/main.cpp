#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "run.hpp"

struct Config {
    size_t width;
    size_t height;
    size_t num_drones;
};

Config read_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file");
    }

    Config config{};
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, delimiter_pos);
        std::string value = line.substr(delimiter_pos + 1);
        
        // Trim whitespace
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        
        if (key == "width") {
            config.width = std::stoul(value);
        } else if (key == "height") {
            config.height = std::stoul(value);
        } else if (key == "numberDrones") {
            config.num_drones = std::stoul(value);
        }
    }
    
    return config;
}

int main() {
    try {
        Config config = read_config("config.txt");
        std::cout << "Starting simulation with:\n"
                  << "Width: " << config.width << "\n"
                  << "Height: " << config.height << "\n"
                  << "Drones: " << config.num_drones << "\n";
        
        run_drone_simulation(config.width, config.height, config.num_drones);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}