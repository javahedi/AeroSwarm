#include <utility>
#include "aeroswarm/parallel/parallel_simulation.hpp"
#include <random>
#include <stdexcept>

ParallelSimulation::ParallelSimulation(ParallelTerrain& terrain,
               std::vector<Drone> drones,
               unsigned int seed) : 
                terrain_(terrain), 
                drones_(std::move(drones)),
                seed_(seed) 
        {
            for (auto& drone : drones_) {
                if(!terrain_.initialize_start_position(drone.position())) {
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


void ParallelSimulation::worker(std::size_t drone_index) {

    Drone& drone = drones_[drone_index];
    // RNG placement is now correct: it is created once per worker, outside the loop, 
    // so each drone continues its own pseudo-random sequence across moves.
    std::mt19937 rng(seed_ + static_cast<unsigned int>(drone_index));

    while (!target_found_.load())
    {
        const auto neighbors =
        terrain_.available_neighbors(drone.position());

        if (neighbors.empty()) {
            return;
        }

        std::uniform_int_distribution<std::size_t> dist(
            0,
            neighbors.size() - 1
        );

        const Position next = neighbors[dist(rng)];

        if (!terrain_.try_claim_cell(next)){
            continue;
        }

        drone.move_to(next);
        if (terrain_.is_target(next)) {
            std::lock_guard<std::mutex> lock(winner_mutex_);

            if (!winning_drone_id_.has_value()) {
                winning_drone_id_ = drone.id();
                target_found_.store(true);
            }

            return;
        }


    }
    
    
};


// void ParallelSimulation::worker(std::size_t drone_index) {

//     Drone& drone = drones_[drone_index];

//     if (target_found_.load()) {
//         return;
//     }

//     const auto neighbors =
//         terrain_.available_neighbors(drone.position());

//     if (neighbors.empty()) {
//         return;
//     }

//     // Each thread has its own RNG object, so there is no shared RNG data race
//     /*
//     seed_ = base simulation seed

//     worker 0 → seed + 0
//     worker 1 → seed + 1
//     worker 2 → seed + 2
//     */
//     // std::mt19937 has internal state. Calling it changes that state
//     std::mt19937 rng(
//         seed_ + static_cast<unsigned int>(drone_index)
//     );

//     std::uniform_int_distribution<std::size_t> dist(
//             0,
//             neighbors.size() - 1
//         );

//     const Position next = neighbors[dist(rng)];

//     if (!terrain_.try_claim_cell(next)){
//         return;
//     }

//     drone.move_to(next);

// }



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