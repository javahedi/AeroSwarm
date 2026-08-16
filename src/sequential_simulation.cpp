
#include <utility>
#include "aeroswarm/sequential/simulation.hpp"

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




SimulationSnapshot Simulation::snapshot() const {
    SimulationSnapshot snapshot;

    snapshot.target_found = target_found_;
    snapshot.winning_drone_id = winning_drone_id_;
    snapshot.tick = tick_;
    snapshot.visited_cells = terrain_.visited_positions();
    snapshot.obstacle_positions = terrain_.obstacle_positions();
    snapshot.target = terrain_.target_position();

    snapshot.drone_positions.reserve(drones_.size());

    for (const auto& drone : drones_) {
        snapshot.drone_positions.push_back(drone.position());
    }

    return snapshot;
}


bool Simulation::step() {
    if (target_found_) {
        return false;
    }

    ++tick_;

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