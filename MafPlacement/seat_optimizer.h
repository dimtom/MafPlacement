#pragma once

#include "metrics.h"
#include "schedule.h"

class SeatOptimizer
{
public:
    SeatOptimizer(
        Schedule& schedule,
        size_t max_iterations,
        std::function<double(const Schedule&)> score_fn)
        : _schedule(schedule)
        , _score_fn(score_fn)
        , _max_iterations(max_iterations)
    {}

public:
    double optimize();

private:
    Schedule& _schedule;
    std::function<double(const Schedule&)> _score_fn;
    size_t _max_iterations;
};