#pragma once
#include <functional>

#include "metrics.h"
#include "schedule.h"

class RandomOptimizer
{
public:
    RandomOptimizer(
        Schedule& schedule, 
        size_t max_iterations,
        std::function<double(const Schedule&)> score_fn)
        : _schedule(schedule)
        , _max_iterations(max_iterations)
        , _score_fn(score_fn)
    {}

    RandomOptimizer() = default;

public:
    double optimize();

    size_t totalIterations() const
    {
        return _total_iterations;
    }
    
    size_t goodIterations() const
    {
        return _good_iterations;
    }

private:
    Schedule& _schedule;
    std::function<double(const Schedule&)> _score_fn;
    size_t _max_iterations;

    size_t _total_iterations;
    size_t _good_iterations;

};

