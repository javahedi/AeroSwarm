#include "aeroswarm/live/sdl_renderer.hpp"

#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

SdlRenderer::SdlRenderer(
    int grid_width,
    int grid_height,
    int cell_size)
    : grid_width_(grid_width),
      grid_height_(grid_height),
      cell_size_(cell_size)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(SDL_GetError());
    }

    if (!TTF_Init()) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    const int terrain_width = grid_width_ * cell_size_;
    const int window_width  = terrain_width + telemetry_width_;
    const int window_height = grid_height_ * cell_size_;

    if (!SDL_CreateWindowAndRenderer(
            "AeroSwarm Live Monitor",
            window_width,
            window_height,
            0,
            &window_,
            &renderer_)) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    font_ = TTF_OpenFont(
        "assets/fonts/DejaVuSans.ttf",
        18.0f
    );

    if (!font_) {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        TTF_Quit();
        SDL_Quit();

        throw std::runtime_error(SDL_GetError());
    }
}

SdlRenderer::~SdlRenderer() {
    if (font_) {
        TTF_CloseFont(font_);
    }

    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

    TTF_Quit();
    SDL_Quit();
}

bool SdlRenderer::process_events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
    }

    return true;
}



void SdlRenderer::draw_telemetry_panel() {
    const float panel_x =
        static_cast<float>(
            grid_width_ * cell_size_
        );

    SDL_FRect panel{
        panel_x,
        0.0f,
        static_cast<float>(telemetry_width_),
        static_cast<float>(
            grid_height_ * cell_size_
        )
    };

    SDL_SetRenderDrawColor(
        renderer_,
        18,
        22,
        28,
        255
    );

    SDL_RenderFillRect(
        renderer_,
        &panel
    );
}


void SdlRenderer::draw_cell(const Position& pos) {
    SDL_FRect rect{
        static_cast<float>(pos.x * cell_size_),
        static_cast<float>(pos.y * cell_size_),
        static_cast<float>(cell_size_),
        static_cast<float>(cell_size_)
    };

    SDL_RenderFillRect(renderer_, &rect);
}


void SdlRenderer::draw_visited(const Position& pos) {
    SDL_SetRenderDrawColor(renderer_, 55, 55, 65, 255);
    draw_cell(pos);
}

void SdlRenderer::draw_obstacle(const Position& pos) {
    SDL_SetRenderDrawColor(renderer_, 110, 90, 70, 255);
    draw_cell(pos);
}

void SdlRenderer::draw_target(const Position& pos) {
    SDL_SetRenderDrawColor(renderer_, 220, 60, 60, 255);
    draw_cell(pos);
}

void SdlRenderer::draw_drone(const Position& pos) {
    SDL_SetRenderDrawColor(renderer_, 60, 160, 230, 255);

    const float margin =
        static_cast<float>(cell_size_) * 0.20f;

    SDL_FRect rect{
        static_cast<float>(pos.x * cell_size_) + margin,
        static_cast<float>(pos.y * cell_size_) + margin,
        static_cast<float>(cell_size_) - 2.0f * margin,
        static_cast<float>(cell_size_) - 2.0f * margin
    };

    SDL_RenderFillRect(renderer_, &rect);
}


void SdlRenderer::draw_grid() {
    SDL_SetRenderDrawColor(renderer_, 70, 70, 70, 255);

    const float width =
        static_cast<float>(grid_width_ * cell_size_);

    const float height =
        static_cast<float>(grid_height_ * cell_size_);

    for (int x = 0; x <= grid_width_; ++x) {
        const float screen_x =
            static_cast<float>(x * cell_size_);

        SDL_RenderLine(
            renderer_,
            screen_x,
            0.0f,
            screen_x,
            height
        );
    }

    for (int y = 0; y <= grid_height_; ++y) {
        const float screen_y =
            static_cast<float>(y * cell_size_);

        SDL_RenderLine(
            renderer_,
            0.0f,
            screen_y,
            width,
            screen_y
        );
    }
}



// void SdlRenderer::render(const SimulationSnapshot& snapshot) {
//     // Clear the entire window first.
//     SDL_SetRenderDrawColor(renderer_, 25, 25, 25, 255);
//     SDL_RenderClear(renderer_);

//     // Draw simulation area.
//     draw_grid();

//     for (const auto& pos : snapshot.visited_cells) {
//         draw_visited(pos);
//     }

//     for (const auto& pos : snapshot.obstacle_positions) {
//         draw_obstacle(pos);
//     }

//     if (snapshot.target.has_value()) {
//         draw_target(snapshot.target.value());
//     }

//     for (const auto& pos : snapshot.drone_positions) {
//         draw_drone(pos);
//     }

//     // Draw telemetry panel AFTER clearing the screen.
//     draw_telemetry_panel();

//     // Show completed frame.
//     SDL_RenderPresent(renderer_);
// }


void SdlRenderer::render(
    const SimulationSnapshot& snapshot)
{
    SDL_SetRenderDrawColor(
        renderer_,
        25,
        25,
        25,
        255
    );

    SDL_RenderClear(renderer_);

    draw_grid();

    for (const auto& pos : snapshot.visited_cells) {
        draw_visited(pos);
    }

    for (const auto& pos : snapshot.obstacle_positions) {
        draw_obstacle(pos);
    }

    if (snapshot.target.has_value()) {
        draw_target(
            snapshot.target.value()
        );
    }

    for (const auto& pos : snapshot.drone_positions) {
        draw_drone(pos);
    }

    // UI layer
    draw_telemetry_panel();
    draw_telemetry(snapshot);

    SDL_RenderPresent(renderer_);
}


void SdlRenderer::draw_text(
    const std::string& text,
    float x,
    float y)
{
    SDL_Color color{
        230,
        235,
        240,
        255
    };

    SDL_Surface* surface =
        TTF_RenderText_Blended(
            font_,
            text.c_str(),
            text.size(),
            color
        );

    if (!surface) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(
            renderer_,
            surface
        );

    if (!texture) {
        SDL_DestroySurface(surface);
        throw std::runtime_error(SDL_GetError());
    }

    SDL_FRect destination{
        x,
        y,
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };

    SDL_DestroySurface(surface);

    SDL_RenderTexture(
        renderer_,
        texture,
        nullptr,
        &destination
    );

    SDL_DestroyTexture(texture);
}


void SdlRenderer::draw_telemetry(
    const SimulationSnapshot& snapshot)
{
    const float panel_x =
        static_cast<float>(
            grid_width_ * cell_size_
        );

    const float left =
        panel_x + 24.0f;

    float y = 30.0f;

    draw_text(
        "AEROSWARM LIVE",
        left,
        y
    );

    y += 45.0f;

    draw_text(
        "STATUS",
        left,
        y
    );

    y += 30.0f;

    draw_text(
        snapshot.target_found
            ? "TARGET FOUND"
            : "SEARCHING",
        left,
        y
    );

    y += 55.0f;

    draw_text(
        "SIMULATION",
        left,
        y
    );

    y += 35.0f;

    draw_text(
        "Tick: " +
        std::to_string(snapshot.tick),
        left,
        y
    );

    y += 28.0f;

    draw_text(
        "Drones: " +
        std::to_string(
            snapshot.drone_positions.size()
        ),
        left,
        y
    );

    y += 28.0f;

    draw_text(
        "Visited: " +
        std::to_string(
            snapshot.visited_cells.size()
        ),
        left,
        y
    );

    y += 55.0f;

    draw_text(
        "TARGET",
        left,
        y
    );

    y += 35.0f;

    if (snapshot.target.has_value()) {
        const Position target =
            snapshot.target.value();

        draw_text(
            "Position: (" +
            std::to_string(target.x) +
            ", " +
            std::to_string(target.y) +
            ")",
            left,
            y
        );
    } else {
        draw_text(
            "Position: --",
            left,
            y
        );
    }

    y += 55.0f;

    draw_text(
        "WINNER",
        left,
        y
    );

    y += 35.0f;

    if (snapshot.winning_drone_id.has_value()) {
        draw_text(
            "Drone #" +
            std::to_string(
                snapshot.winning_drone_id.value()
            ),
            left,
            y
        );
    } else {
        draw_text(
            "--",
            left,
            y
        );
    }
}