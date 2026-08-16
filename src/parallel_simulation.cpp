#include <utility>
#include "aeroswarm/parallel/simulation.hpp"
#include <random>
#include <stdexcept>

ParallelSimulation::ParallelSimulation(
                ParallelTerrain& terrain,
                std::vector<Drone> drones,
                unsigned int seed,
                std::chrono::milliseconds update_interval)
                : terrain_(terrain),
                drones_(std::move(drones)),
                seed_(seed),
                update_interval_(update_interval)
            {
                for (const auto& drone : drones_) {
                    if (!terrain_.initialize_start_position(drone.position())) {
                        throw std::invalid_argument("Invalid drone start position");
                    }
                }
            }


bool ParallelSimulation::target_found() const {
    return target_found_.load();

}
std::optional<int> ParallelSimulation::winning_drone_id() const {
    std::lock_guard<std::mutex> lock(winner_mutex_);
    return winning_drone_id_;
}



/*
Worker 1 ──┐
Worker 2 ──┼── modifying simulation
Worker 3 ──┘
              │
              │ concurrently
              ▼
          snapshot()
              │
              ▼
         renderer @ 60 FPS

    >>>> our different pieces of shared state <<<<

            Terrain/grid
                protected by ParallelTerrain::mtx_

            Drone positions (shared_mutex:: shared_mutex because we 
                            have two kinds of access :: read/write)
                protected by ParallelSimulation::drones_mutex_


                                  drones_mutex_

                    snapshot()       worker #2 read
                    shared_lock      shared_lock
                        \              /
                        \            /
                            both allowed

                                BUT

                            worker moves drone
                            unique_lock
                                |
                        everyone else waits







            Winner ID
                protected by ParallelSimulation::winner_mutex_

            target_found + tick
                atomic

*/

void ParallelSimulation::worker(std::size_t drone_index) {

    std::mt19937 rng(
        seed_ + static_cast<unsigned int>(drone_index)
    );



    int drone_id;

    {
        std::shared_lock<std::shared_mutex> lock(drones_mutex_);
        drone_id = drones_[drone_index].id();
    }


    auto next_update = std::chrono::steady_clock::now();
    while (!target_found_.load()) {

        if (update_interval_.count() > 0) {
            next_update += update_interval_;
            std::this_thread::sleep_until(next_update);
        }

        Position current_position;

        // READ drone state safely
        {
            //I only want to read. Other readers may read at the same time.
            std::shared_lock<std::shared_mutex> lock(drones_mutex_);
            current_position = drones_[drone_index].position();
        }

        // Terrain has its own mutex internally
        const auto neighbors =
            terrain_.available_neighbors(current_position);

        if (neighbors.empty()) {
            return;
        }

        std::uniform_int_distribution<std::size_t> dist(
            0,
            neighbors.size() - 1
        );

        const Position next = neighbors[dist(rng)];

        // Another drone may have claimed it since available_neighbors()
        if (!terrain_.try_claim_cell(next)) {
            continue;
        }

        // WRITE drone state safely
        {
            // in share_mute, when we write we use unique_lock
            // I am writing. Nobody else may read or write this protected state while I do it.
            std::unique_lock<std::shared_mutex> lock(drones_mutex_);
            drones_[drone_index].move_to(next);
        }

        tick_.fetch_add(1);

        if (terrain_.is_target(next)) {

            std::lock_guard<std::mutex> lock(winner_mutex_);

            if (!winning_drone_id_.has_value()) {
                winning_drone_id_ = drone_id;
                target_found_.store(true);
            }

            return;
        }
    }
}


SimulationSnapshot ParallelSimulation::snapshot() const {
    SimulationSnapshot snapshot;

    snapshot.target_found = target_found_.load();
    snapshot.tick = tick_.load();

    {
        std::lock_guard<std::mutex> lock(winner_mutex_);
        snapshot.winning_drone_id = winning_drone_id_;
    }


    // renderer’s snapshot also only reads:
    {
        std::shared_lock<std::shared_mutex> lock(drones_mutex_);

        snapshot.drone_positions.reserve(drones_.size());

        for (const auto& drone : drones_) {
            snapshot.drone_positions.push_back(
                drone.position()
            );
        }
    }

    snapshot.visited_cells = terrain_.visited_positions();

    snapshot.obstacle_positions = terrain_.obstacle_positions();

    snapshot.target = terrain_.target_position();

    return snapshot;
}



ParallelSimulationStatus ParallelSimulation::run() {
    std::vector<std::thread> threads;
    /*
    
    Thread 0 -> this->worker(0)
    Thread 1 -> this->worker(1)
    Thread 2 -> this->worker(2)
    */

    for (std::size_t i = 0; i < drones_.size(); ++i) {
        threads.emplace_back(
            &ParallelSimulation::worker, // member function to execute. 
            this,                        // call that member fucntion of ParallelSimulation
            i                            // agument pass to woker
        );
    }

    for (auto& thread : threads) {
        thread.join();
    }

    if (target_found_.load()) {
        return ParallelSimulationStatus::TargetFound;
    }

    return ParallelSimulationStatus::Stuck;
}