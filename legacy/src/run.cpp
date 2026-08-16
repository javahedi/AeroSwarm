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


void run_drone_simulation(size_t width, size_t height, size_t num_drones) {
    //std::cout << "Creating terrain...\n";  // DEBUG
   
    Terrain terrain(width, height);


    //std::cout << "Initializing drones...\n";  // DEBUG
    // Initialize drone at a random position
    std::random_device rd;
    std::mt19937 gen(rd());
    
    
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
        drones.emplace_back(i, pos, terrain); 
        //std::cout << "Drone " << i << " placed at (" << pos.x << "," << pos.y << ")\n";  // DEBUG


    }
    //std::cout << "Drones initialized at random positions.\n";  // DEBUG


    create_frames_directory("frames");
    int iterations = 0;
    std::ostringstream oss;

  

    std::cout << "Starting simulation loop...\n"; 
    // loop over all drones 
    // Create an instance of SimulationControl
    SimulationControl simControl;
    while (!simControl.target_found) {
        for (auto& drone : drones) {

            if (drone.has_reached_target()) {
                simControl.target_found = true;
                simControl.winning_drone_id = drone.get_id(); // Set the winning drone ID
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
            }

            // Check if the drone has reached the target  
            if (simControl.winning_drone_id >= 0) {
                std::cout << "🎯 Drone #" << simControl.winning_drone_id
                        << " found the target at ("
                        << terrain.target.x << "," << terrain.target.y << ")\n";
            }

    }
}


