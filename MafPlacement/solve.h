#pragma once

#include <memory>

#include "configuration.h"
#include "schedule.h"

std::unique_ptr<Schedule> solvePlayers(
    const Configuration& conf,
    size_t num_stages,
    size_t num_iterations);

std::unique_ptr<Schedule> solveSeats(
    const Schedule& schedule,
    size_t num_stages,
    size_t num_iterations);



