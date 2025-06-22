#include "run.hpp"
#include "utility.hpp"
#include "terrain.hpp"
#include "drone.hpp"
#include <filesystem>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>



namespace fs = std::filesystem;


void run_drone_simulation() {
    //std::cout << "Creating terrain...\n";  // DEBUG
    const int width = 30;
    const int height = 50;
    Terrain terrain(width, height);


    //std::cout << "Initializing drones...\n";  // DEBUG
    // Initialize drone at a random position
    std::random_device rd;
    std::mt19937 gen(rd());
    
    const size_t num_drones = 4;
    std::vector<Drone> drones;
    for (size_t i = 0; i < num_drones; ++i) {
        Position pos;
        //int attempts = 0;  // DEBUG
        while(true) {
            //attempts++;  // DEBUG
            //if (attempts > 100) {  // DEBUG
            //    std::cerr << "Failed to find a valid position for drone after 100 attempts.\n";
            //    break;
            //}
            pos.x = std::uniform_int_distribution<>(0, width - 1)(gen);
            pos.y = std::uniform_int_distribution<>(0, height - 1)(gen);
            if (terrain.is_valid_position(pos)) {
                break;
            }
        }
        /*
        Situation	                                  |     Recommended
        ------------------------------------------------------------------
        You have an existing object	                  |      push_back
        You want to construct the object in place	  |      emplace_back
        */
        drones.emplace_back(pos, terrain); 
        //std::cout << "Drone " << i << " placed at (" << pos.x << "," << pos.y << ")\n";  // DEBUG


    }
    //std::cout << "Drones initialized at random positions.\n";  // DEBUG


    create_frames_directory("frames");
    int iterations = 0;
    std::ostringstream oss;



    std::cout << "Starting simulation loop...\n"; 
    // loop over all drones
    bool target_found = false;  
    while (!target_found) {
        for (auto& drone : drones) {

            if (drone.has_reached_target()) {
                target_found = true;
                break;
            }

            try {
                Position next_pos = drone.find_next_position();
                drone.move_to(next_pos);
                iterations++;
                oss.str(""); // Clear stringstream
                oss << "frames/frame_" << std::setw(4) << std::setfill('0') << iterations << ".txt";
                //drones[0].save_terrain_to_file(oss.str()); 
                drone.save_terrain_to_file(oss.str());

                std::cout << "Iteration: " << iterations << std::endl;
                if (iterations > 1000) {
                    std::cout << "Drone has been running for too long, exiting simulation." << std::endl;
                    break;
                }
                } catch (const std::runtime_error& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    break;
                }
            // Check if the drone has reached the target
            if (drone.has_reached_target()) {
                std::cout << " 💥 💥 Drone has reached the target! 💥 💥 " << std::endl;
                break;
            }
        }
    }
}


