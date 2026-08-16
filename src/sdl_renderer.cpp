#include "aeroswarm/live/sdl_renderer.hpp"

#include <stdexcept>

#include <SDL3/SDL.h>

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

    const int window_width  = grid_width_ * cell_size_;
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
}

SdlRenderer::~SdlRenderer() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

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


void SdlRenderer::render(const SimulationSnapshot& snapshot) {
    SDL_SetRenderDrawColor(renderer_, 25, 25, 25, 255);
    SDL_RenderClear(renderer_);

    draw_grid();

    for (const auto& pos : snapshot.visited_cells) {
        draw_visited(pos);
    }

    for (const auto& pos : snapshot.obstacle_positions) {
        draw_obstacle(pos);
    }

    if (snapshot.target.has_value()) {
        draw_target(snapshot.target.value());
    }

    for (const auto& pos : snapshot.drone_positions) {
        draw_drone(pos);
    }

    SDL_RenderPresent(renderer_);
}