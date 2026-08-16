#pragma once

#include <string>

#include "aeroswarm/live/simulation_snapshot.hpp"

class SdlRenderer {
public:
    SdlRenderer(
        int grid_width,
        int grid_height,
        int cell_size = 20
    );

    ~SdlRenderer();

    // No copying: this object will own SDL resources.
    SdlRenderer(const SdlRenderer&) = delete;
    SdlRenderer& operator=(const SdlRenderer&) = delete;

    // Process window events.
    // Returns false when the user closes the window.
    bool process_events();

    // Render one immutable simulation snapshot.
    void render(const SimulationSnapshot& snapshot);

private:
    int grid_width_;
    int grid_height_;
    int cell_size_;

    struct SDL_Window* window_{nullptr};
    struct SDL_Renderer* renderer_{nullptr};

    void draw_grid();
    void draw_cell(const Position& pos);
    void draw_obstacle(const Position& pos);
    void draw_target(const Position& pos);
    void draw_drone(const Position& pos);
    void draw_visited(const Position& pos);

};