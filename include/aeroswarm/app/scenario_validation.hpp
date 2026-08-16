#pragma once

#include <string>

#include "aeroswarm/app/scenario.hpp"

bool validate_scenario(
    const Scenario& scenario,
    std::string& error_message
);