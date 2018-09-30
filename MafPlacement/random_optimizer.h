#pragma once
#include <functional>

#include "metrics.h"
#include "schedule.h"

class RandomOptimizer
{
public:
    RandomOptimizer(Schedule& schedule, 
        std::function<double(const Schedule&, Metrics&)> score_fn,
        size_t max_iterations)
        : _schedule(schedule)
        , _score_fn(score_fn)
        , _max_iterations(max_iterations)
    {}

    RandomOptimizer() = default;

public:
    double optimize();

private:
    Schedule& _schedule;
    std::function<double(const Schedule&, Metrics&)> _score_fn;
    size_t _max_iterations;

};

