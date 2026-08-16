#pragma once

#include <atomic>
#include <optional>
#include <thread>
#include <random>
#include <vector>
#include <mutex>

#include "aeroswarm/drone.hpp"
#include "aeroswarm/parallel/parallel_terrain.hpp"



enum class ParallelSimulationStatus {
    TargetFound,
    Stuck
};

class ParallelSimulation {
    public:
        ParallelSimulation(ParallelTerrain& terrain,
                            std::vector<Drone> drones,
                            unsigned int seed);

        ParallelSimulationStatus run();

        bool target_found() const;
        std::optional<int> winning_drone_id() const;



    private:
        /*
        ParallelTerrain contains a std::mutex, and mutexes are not copyable. so we use &
        Also, all drone workers must operate on the same shared terrain, not copies.
        */
        ParallelTerrain& terrain_; 
        std::vector<Drone> drones_;
        unsigned int seed_;

        /*
        Provides lock-free synchronization for many primitive data types.
        Enables safe communication between multiple threads without explicit mutexes.
        */
        // Shared stop flag safely read/written by multiple worker threads.
        std::atomic<bool> target_found_{false};
        // We'll discuss synchronization for this before implementing it.
        std::optional<int> winning_drone_id_;
        mutable std::mutex winner_mutex_;


        void worker(std::size_t drone_index);
};