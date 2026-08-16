#include "aeroswarm/app/parallel_live_runner.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <functional>

#include "aeroswarm/parallel/simulation.hpp"
#include "aeroswarm/parallel/terrain.hpp"

/*
Live execution model

Simulation workers:
    ~100 Hz
    one movement opportunity every 10 ms

Renderer / monitor:
    ~60 FPS
    one snapshot approximately every 16 ms

The simulation and renderer intentionally run at different rates.

Why?

The simulation is the PRODUCER:
    it updates world state frequently.

The monitor is the CONSUMER:
    it does not need to draw every simulation update.
    It only needs the latest safe snapshot when a new frame is rendered.

Example:

    simulation ticks:
    0 1 2 3 4 5 6 7 8 9 ...

    rendered frames:
    0   2   4   5   7   9 ...

Some simulation states are never displayed, which is normal for a
real-time monitor.

This keeps simulation timing independent from rendering performance
and gives us an architecture that can later support SDL3 or a network
telemetry producer without changing the simulation model.


time(ms)     0   10   20   30   40   50   60
simulation   S    S    S    S    S    S    S (100Hz=10ms)

render       R         R         R         R (60Hz=16ms)



MAIN THREAD                         SIMULATION THREAD
-----------                         -----------------

create simulation_thread
       │
       ├────────────────────────────→ simulation.run()
       │                                  │
       │                                  │
       ▼                                  │
while (!simulation_finished)              │
       │                                  │
snapshot()                                │
print                                     │
sleep 16ms                                │
       │                                  │
snapshot()                                │
print                                     │
sleep 16ms                                │
       │                                  │
       │                              simulation ends
       │                                  │
       │                         simulation_finished=true
       │                                  │
       ▼                                  ▼
loop stops
       │
simulation_thread.join()



MAIN / MONITOR THREAD              SIMULATION THREAD

simulation_finished = false

start simulation_thread ----------> simulation.run()
        |
        v
while (!false)
        |
        v
snapshot()
print
sleep
        |
        v
while (!false)
        |
        v
snapshot()
print
sleep
                                     simulation finishes
                                             |
                                             v
                                simulation_finished.store(true)

while (!true)
        |
        v
false -> exit loop
*/


void run_simulation_thread(
    ParallelSimulation& simulation,
    ParallelSimulationStatus& final_status,
    std::atomic<bool>& simulation_finished)
{
    final_status = simulation.run();
    simulation_finished.store(true);
}


int run_parallel_live(const Scenario& scenario) {

    // Build the shared terrain used by all parallel drone workers.
    ParallelTerrain terrain{
        scenario.width,
        scenario.height
    };

    terrain.set_target(scenario.target);

    // Apply the same scenario obstacles used by the other runners.
    for (const auto& obstacle : scenario.obstacles) {
        terrain.set_obstacle(obstacle);
    }

    /*
    Simulation pacing: 10 ms between worker updates.

        1000 ms / 10 ms = 100 updates per second

    This prevents the simulation from finishing in a few milliseconds,
    which would make a live monitor unable to observe intermediate state.
    */
    constexpr auto simulation_interval =
        std::chrono::milliseconds{10};

    ParallelSimulation simulation{
        terrain,
        scenario.drones,
        scenario.seed,
        simulation_interval
    };

    /*
    Shared completion flag between:

        simulation thread
        live monitor thread

    Atomic is used because one thread writes the flag while another
    repeatedly reads it.
    */
    std::atomic<bool> simulation_finished{false};

    // Written by the simulation thread and read only after join().
    ParallelSimulationStatus final_status{
        ParallelSimulationStatus::Stuck
    };


    /*
    Run the simulation in a background thread.

    This allows the current thread to remain responsible for the live
    monitor / renderer.
    */

    // Normal
    // std::thread simulation_thread(
    //     run_simulation_thread,
    //     std::ref(simulation),
    //     std::ref(final_status),
    //     std::ref(simulation_finished)
    // );
    // Lambda >> [&]: lambda can use surrounding 
    // variables(final_status,simulation,simulation_finished) by reference
    std::thread simulation_thread([&]() {
        final_status = simulation.run();

        // Publish completion to the monitor loop.
        simulation_finished.store(true); // wrtie to attomic
    });


    /*
    Rendering cadence:

        ~16 ms/frame
        1000 / 16 ≈ 62.5 FPS

    This is an approximation of 60 FPS.

    Later SDL rendering will use this same consumer loop.
    */
    constexpr auto frame_time = std::chrono::milliseconds{16};

    /*
    Live monitor loop.

    While the simulation runs:
        1. request a thread-safe snapshot
        2. display the latest state
        3. sleep until approximately the next frame

    The monitor never directly accesses worker-owned mutable state.
    */
    while (!simulation_finished.load()) { // read from atomic

        const auto snapshot = simulation.snapshot();

        std::cout
            << "\n"
            << "tick=" << snapshot.tick
            << " drones=" << snapshot.drone_positions.size()
            << " visited=" << snapshot.visited_cells.size()
            << " target_found="
            << (snapshot.target_found ? "true" : "false")
            << std::flush;

        std::this_thread::sleep_for(frame_time);
    }

    /*
    The simulation thread must be joined before leaving this function.

    join() guarantees:
        - the worker thread has finished
        - final_status has been written
        - no background simulation thread survives this scope
    */
    simulation_thread.join();

    // Capture the final stable state after all workers have finished.
    const auto snapshot = simulation.snapshot();

    std::cout
        << '\n'
        << "Final tick: "
        << snapshot.tick
        << '\n';

    if (final_status == ParallelSimulationStatus::TargetFound) {

        std::cout
            << "Parallel live simulation: target found\n";

        if (snapshot.winning_drone_id.has_value()) {
            std::cout
                << "Winning drone: #"
                << snapshot.winning_drone_id.value()
                << '\n';
        }

    } else {

        std::cout
            << "Parallel live simulation: stuck\n";
    }

    return 0;
}