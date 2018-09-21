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
    calcPlayerSeatsHistogram(int num_player);

    // histogram of player's opponents
    // returns vector of size "number of players"
    std::vector<int>
    calcPlayerOpponentsHistogram(int player_id);

public:
    static int calcMin(const std::vector<int>& v);
    static int calcMax(const std::vector<int>& v);
    static double calcAverage(const std::vector<int>& v);
    static double calcSquareDeviation(const std::vector<int>& v);
    static double calcSquareDeviation(const std::vector<int>& v, double target);

public:
    const Schedule& _schedule;

};
