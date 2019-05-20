#include "random_optimizer.h"
#include "metrics.h"

double RandomOptimizer::optimize()
{
    const auto& conf = _schedule.config();

    // modify schedule
    size_t num_probes = 0;
    _total_iterations = 0;
    _good_iterations = 0;

    for (size_t i = 0; i < _max_iterations; i++)
    {
        bool success = _schedule.randomSeatChange([&]() {
            num_probes++;
            return _score_fn(_schedule);
        });
        _good_iterations += (int)success;
        _total_iterations++;
    }

    return _score_fn(_schedule);
}