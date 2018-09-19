#pragma once
#include "schedule.h"

class Metrics
{
public:
    Metrics(const Schedule& schedule)
        : _schedule(schedule)
    {}

    ~Metrics() = default;

public:
    // histogram of player seats
    // returns vector of size "10"
    std::vector<int> 
    CalcPlayerSeatsHistogram(int num_player);

    // histogram of player's opponents
    // returns vector of size "number of players"
    std::vector<int>
    CalcPlayerOpponentsHistogram(int player_id);

public:
    const Schedule& _schedule;

};
