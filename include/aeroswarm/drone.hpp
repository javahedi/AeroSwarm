#pragma once

#include "aeroswarm/types.hpp"

class Drone {
public:
    Drone(int id, Position start_position)
        : id_(id),
          position_(start_position)
    {
    }

    int id() const {
        return id_;
    }

    const Position& position() const {
        return position_;
    }

    void move_to(const Position& new_position) {
        position_ = new_position;
    }

private:
    int id_;
    Position position_;
};