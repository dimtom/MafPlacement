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
    calcPlayerSeatsHistogram(player_t player_id);

    // histogram of player's opponents
    // returns vector of size "number of players"
    std::vector<int>
    calcPlayerOpponentsHistogram(player_t player_id);

public:
    static int calcMin(const std::vector<int>& v, size_t exclude_idx);
    static int calcMax(const std::vector<int>& v, size_t exclude_idx);
    static double calcAverage(const std::vector<int>& v, size_t exclude_idx);
    static double calcSquareDeviation(const std::vector<int>& v, size_t exclude_idx);
    static double calcSquareDeviation(const std::vector<int>& v, size_t exclude_idx, double target);

public:
    const Schedule& _schedule;

};
