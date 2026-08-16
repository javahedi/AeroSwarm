#pragma once

#include <random>
#include <vector>
#include <optional>

#include "aeroswarm/drone.hpp"
#include "aeroswarm/sequential/terrain.hpp"
#include "aeroswarm/live/simulation_snapshot.hpp"


enum class SimulationStatus {
    Running,
    TargetFound,
    Stuck
};


/*
seed>>> std::mt19937 rng_{42};>>
will produce the same pseudo-random sequence every run.
That gives us:
>>> 
random behavior
+
repeatable test
=
deterministic simulation
*/
class Simulation {
public:
    // Simulation can take ownership of its own copy/moved state
    Simulation(Terrain terrain,
               std::vector<Drone> drones,
               unsigned int seed);

    bool step();
    const Terrain& terrain() const;
    const std::vector<Drone>& drones() const;
    bool target_found() const;
    //int winning_drone_id() const;
    const std::optional<int>& winning_drone_id() const;

    SimulationStatus run_until_done();
    SimulationSnapshot snapshot() const;

private:
    Terrain terrain_;
    std::vector<Drone> drones_;
    std::mt19937 seed_;
    bool target_found_{false};
    //int winning_drone_id_{-1};
    std::optional<int> winning_drone_id_;
    std::size_t tick_{0};
};