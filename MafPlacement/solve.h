#pragma once

#include <memory>

#include "configuration.h"
#include "schedule.h"

std::unique_ptr<Schedule> solvePlayers(
    const Configuration& conf,
    size_t player_step,
    size_t num_stages,
    size_t num_iterations,
    std::function<bool(const Schedule&, double)> schedule_fn);

std::unique_ptr<Schedule> solveSeats(
    const Schedule& schedule,
    size_t num_attempts,
    size_t num_stages,
    size_t shuffle_per_stage,
    size_t switch_per_stage,
    std::function<bool(const Schedule&, double)> schedule_fn);



