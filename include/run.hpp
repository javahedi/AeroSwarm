
#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>


struct SimulationControl {
    std::atomic<bool> target_found{false};
    std::mutex terrain_mutex;
    std::mutex output_mutex;
    int frames_count = 0;
};


void run_drone_simulation();
