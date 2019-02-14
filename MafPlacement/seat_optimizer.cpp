#include "seat_optimizer.h"

double SeatOptimizer::optimize()
{
    size_t div = 100;

    size_t good_iterations = 0;
    for (size_t i = 0; i < _max_iterations; i++) {
        size_t game_idx = _schedule.generateRandomGame();
        size_t seat_one = _schedule.generateRandomSeat();
        size_t seat_two = _schedule.generateRandomSeat();

        if (seat_one == seat_two) {
            continue;
        }

        /*if ((i % div) == 0) {
            auto score = _score_fn(_schedule);
            printf("Iteration #%zu: score=%6.2f good iterations: %zu\n", i, score, good_iterations);
            div *= 5;
        }*/

        auto score_before = _score_fn(_schedule);
        _schedule.switchSeats(game_idx, seat_one, seat_two);
        auto score_after = _score_fn(_schedule);

        if (score_after >= score_before) {
            // roll back if score is worse than before
            _schedule.switchSeats(game_idx, seat_two, seat_one);
        }
        else {
            good_iterations++;
        }
    }

    auto score = _score_fn(_schedule);
    return score;
}