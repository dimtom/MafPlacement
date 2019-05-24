#pragma once

#include "metrics.h"
#include "schedule.h"

class SeatOptimizer
{
public:
    SeatOptimizer(
        Schedule& schedule,
        size_t max_stages,
        size_t max_shuffles,
        size_t max_switches,
        std::function<double(const Schedule&)> score_fn)
        : _schedule(schedule)
        , _score_fn(score_fn)
        , _max_stages(max_stages)
        , _shuffle_per_stage(max_shuffles)
        , _switch_per_stage(max_switches)
    {}

public:
    double optimize();

private:
    bool shuffleGame(size_t game_idx);
    bool shuffleGames(size_t game_idx_one, size_t game_idx_two);
    bool switchTwoSeats(size_t game_idx);

private:
    Schedule& _schedule;
    std::function<double(const Schedule&)> _score_fn;

    size_t _max_stages;
    size_t _shuffle_per_stage;
    size_t _switch_per_stage;
};