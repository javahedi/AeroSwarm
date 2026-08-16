#include "aeroswarm/app/parallel_live_runner.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "aeroswarm/parallel/simulation.hpp"
#include "aeroswarm/parallel/terrain.hpp"

/*

Parallel worker(s)
   ~100 Hz
      │
      │ thread-safe snapshot()
      ▼
Live monitor
   ~60 FPS
   
*/

int run_parallel_live(const Scenario& scenario) {
    ParallelTerrain terrain{
        scenario.width,
        scenario.height
    };

    terrain.set_target(scenario.target);

    for (const auto& obstacle : scenario.obstacles) {
        terrain.set_obstacle(obstacle);
    }

    constexpr auto simulation_interval =
                std::chrono::milliseconds{10};

    ParallelSimulation simulation{
        terrain,
        scenario.drones,
        scenario.seed,
        simulation_interval
    };

    std::atomic<bool> simulation_finished{false};

    ParallelSimulationStatus final_status{
        ParallelSimulationStatus::Stuck
    };

    std::thread simulation_thread([&]() {
        final_status = simulation.run();
        simulation_finished.store(true);
    });

    constexpr auto frame_time =
        std::chrono::milliseconds{16}; // roughly 60 FPS

    while (!simulation_finished.load()) {
        const auto snapshot = simulation.snapshot();

        std::cout
            << "\r"
            << "tick=" << snapshot.tick
            << " drones=" << snapshot.drone_positions.size()
            << " visited=" << snapshot.visited_cells.size()
            << " target_found="
            << (snapshot.target_found ? "true" : "false")
            << std::flush;

        std::this_thread::sleep_for(frame_time);
    }

    simulation_thread.join();





    // One final snapshot after simulation completion.
    const auto snapshot = simulation.snapshot();

    std::cout << '\n'
          << "Final tick: " << snapshot.tick
          << '\n';

    std::cout << '\n';

    if (final_status == ParallelSimulationStatus::TargetFound) {
        std::cout << "Parallel live simulation: target found\n";

        if (snapshot.winning_drone_id.has_value()) {
            std::cout
                << "Winning drone: #"
                << snapshot.winning_drone_id.value()
                << '\n';
        }
    } else {
        std::cout << "Parallel live simulation: stuck\n";
    }

    return 0;
}