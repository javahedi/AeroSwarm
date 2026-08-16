
#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>


struct SimulationControl {
    std::atomic<bool> target_found{false};
    std::atomic<int> winning_drone_id{-1}; // ID of the drone that found the target
    std::mutex terrain_mutex;
    std::mutex output_mutex;
    int frames_count = 0;
};


void run_drone_simulation(size_t width, size_t height, size_t num_drones);
