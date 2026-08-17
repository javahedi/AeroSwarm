#include "aeroswarm/app/parallel_sdl_runner.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "aeroswarm/live/sdl_renderer.hpp"
#include "aeroswarm/parallel/simulation.hpp"
#include "aeroswarm/parallel/terrain.hpp"

/*
SDL live mode

Simulation:
    background thread
    ~100 Hz update cadence

SDL monitor:
    main thread
    ~60 FPS rendering cadence

Important:
SDL window creation, event polling, and rendering stay on the main thread.
The simulation runs independently in the background.


                 SDL MAIN THREAD                 SIMULATION THREAD

window opens
     │
     ├────────────────────────────────────────→ simulation.run()
     │                                              │
     │ snapshot → render                           move
     │ snapshot → render                           move
     │ snapshot → render                           move
     │                                              │
     │                                      simulation finishes
     │                                              │
     │                                  simulation_finished = true
     │
     │ sees finished
     │
     │ join()
     │
     │ snapshot final state
     │ render final state
     │ snapshot final state
     │ render final state
     │ ...
     │
USER CLOSES WINDOW
     │
     ▼
program ends
*/

int run_parallel_sdl(const Scenario& scenario) {
    ParallelTerrain terrain{
        scenario.width,
        scenario.height
    };

    terrain.set_target(scenario.target);

    for (const auto& obstacle : scenario.obstacles) {
        terrain.set_obstacle(obstacle);
    }

    /*
    Worker update cadence:

        10 ms/update
        ~= 100 Hz

    The simulation produces state faster than the screen renders it.
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
    SDL resources are created on this thread.

    This thread remains responsible for:
        - SDL event handling
        - snapshots
        - rendering

    SDL documents window creation and event polling as main-thread
    operations. 
    */
    SdlRenderer renderer{
        scenario.width,
        scenario.height,
        20
    };

    /*
    Communication flag:

    simulation thread writes:
        false -> true

    main/render thread reads it each frame.
    */
    std::atomic<bool> simulation_finished{false};

    ParallelSimulationStatus final_status{
        ParallelSimulationStatus::Stuck
    };

    /*
    Background simulation thread.

    After simulation.run() returns, all internal drone workers have
    completed. The thread then publishes completion through the atomic
    flag.
    */
    std::thread simulation_thread([&]() {
        final_status = simulation.run();
        simulation_finished.store(true);
    });

    /*
    Render cadence:

        ~16 ms/frame
        ~= 60 FPS

    The renderer does not need every simulation tick.
    It renders the newest available snapshot.
    */
    constexpr auto frame_time =
        std::chrono::milliseconds{16};

    bool window_open = true;

    bool simulation_joined = false;

    while (window_open) {

        // SDL event handling stays on the main/render thread.
        window_open = renderer.process_events();

        if (!window_open) {
            break;
        }

        /*
        If the simulation has finished, join its controlling thread once.

        After this point the simulation state is final and stable,
        but we KEEP rendering it until the user closes the window.
        */
        if (simulation_finished.load() && !simulation_joined) {
            simulation_thread.join();
            simulation_joined = true;
        }

        // While running: latest live state.
        // After finishing: final frozen state.
        const auto snapshot = simulation.snapshot();

        renderer.render(snapshot);

        std::this_thread::sleep_for(frame_time);
    }
    /*
    Important:

    If the simulation finished naturally, join() returns immediately or
    very soon.

    If the user closes the SDL window BEFORE the simulation finishes,
    this current design still waits for the simulation to finish.

    Later we can add a cancellation/stop request.
    */
    /*
    If the user closed the window while the simulation was still running,
    we still need to join the simulation thread before destroying simulation.
    */
    if (!simulation_joined) {
        simulation_thread.join();
    }

    const auto final_snapshot = simulation.snapshot();

    if (final_status == ParallelSimulationStatus::TargetFound) {
        std::cout << "Parallel SDL simulation: target found\n";

        if (final_snapshot.winning_drone_id.has_value()) {
            std::cout
                << "Winning drone: #"
                << final_snapshot.winning_drone_id.value()
                << '\n';
        }
    } else {
        std::cout << "Parallel SDL simulation: stuck\n";
    }

    std::cout
        << "Final tick: "
        << final_snapshot.tick
        << '\n';


   // Allocation instrumentation summary
    //  include/aeroswarm/parallel/terrain.hpp
    // if you want use 
    // std::vector<Position> available_neighbors_vector(const Position& pos) const
    // instead of heap free
    //  Neighbors available_neighbors(const Position& pos) const
    // std::cout
    //     << "Neighbor calls: "
    //     << terrain.neighbor_calls()
    //     << '\n'
    //     << "Neighbor vector growths: "
    //     << terrain.neighbor_capacity_growths()
    //     << '\n';

    return 0;
}