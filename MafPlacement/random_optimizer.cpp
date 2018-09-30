#include "random_optimizer.h"
#include "metrics.h"

double RandomOptimizer::optimize()
{
    Metrics metrics(_schedule);
    const auto& conf = _schedule.config();

    // modify schedule
    int div = 30;
    size_t num_probes = 0;
    size_t num_iterations = 0;
    size_t num_good = 0;

    for (size_t i = 0; i < _max_iterations; i++)
    {
        if ((i % div) == 0) {
            auto score = _score_fn(_schedule, metrics);
            printf("Iteration #%zu: score=%6.2f good iterations: %zu\n", i, score, num_good);
            div *= 5;
        }

        bool success = _schedule.randomSeatChange([&]() {
            num_probes++;
            return _score_fn(_schedule, metrics);
        });
        num_good += (int)success;
        num_iterations++;
    }

    printf("Number of iterations: %zu\n", num_iterations);
    printf("Number of probes: %zu\n", num_probes);
    printf("Good iterations: %zu\n", num_good);

    return _score_fn(_schedule, metrics);
}