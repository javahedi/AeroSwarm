
#include <utility>
#include "aeroswarm/simulation.hpp"


Simulation::Simulation(Terrain terrain,
               std::vector<Drone> drones,
               unsigned int seed) : 
                terrain_(std::move(terrain)), 
                drones_(std::move(drones)),
                seed_(seed) 
        {
            for (const auto& drone : drones_) {
                terrain_.mark_visited(drone.position());
            }

        }


const Terrain& Simulation::terrain() const {
    return terrain_;
}

const std::vector<Drone>& Simulation::drones() const {
    return drones_;
}


bool Simulation::target_found() const{
    return target_found_;
}

const std::optional<int>&  Simulation::winning_drone_id() const{
    return winning_drone_id_;
}


bool Simulation::step() {
    if (target_found_) {
        return false;
    }

    bool moved_any = false;

    for (auto& drone : drones_) {
        auto neighbors = terrain_.available_neighbors(drone.position());

        if (neighbors.empty()) {
            continue;
        }

        std::uniform_int_distribution<std::size_t> dist(
            0,
            neighbors.size() - 1
        );

        const Position next = neighbors[dist(seed_)];

        drone.move_to(next);
        terrain_.mark_visited(next);
        moved_any = true;

        if (terrain_.cell_at(next).type == CellType::Target) {
            target_found_ = true;
            winning_drone_id_ = drone.id();
            break;
        }
    }

    return moved_any;
};


SimulationStatus Simulation::run_until_done() {
    while (true) {
        if (target_found_) {
            return SimulationStatus::TargetFound;
        }

        const bool moved = step();

        if (target_found_) {
            return SimulationStatus::TargetFound;
        }

        if (!moved) {
            return SimulationStatus::Stuck;
        }
    }
};