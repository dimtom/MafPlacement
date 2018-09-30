#include "metrics.h"
#include <cassert>

// histogram of player seats
std::vector<int> 
Metrics::calcPlayerSeatsHistogram(player_t player_id)
{
    std::vector<int> player_seats(Configuration::NumSeats, 0);
    for (const auto& game : _schedule.games())
    {
        if (!game.participates(player_id)) {
            continue;
        }

        seat_t pos = game.players()[player_id];
        player_seats[pos]++;
    }

    return player_seats;
}

// histogram of player opponents
std::vector<int>
Metrics::calcPlayerOpponentsHistogram(player_t player_id)
{
    std::vector<int> player_opponents(_schedule.config().numPlayers(), 0);
    for (const auto& game : _schedule.games())
    {
        if (!game.participates(player_id)) {
            continue;
        }

        for (auto id : game.seats()) {
            player_opponents[id]++;
        }
    }

    return player_opponents;
}

double Metrics::aggregate(const std::vector<int>& v, size_t exclude_idx, std::function<double(int)> fn)
{
    assert(v.size() > 1);

    double accumulator = 0.0;
    for (int i = 0; i < v.size(); i++)
        if (i != exclude_idx)
            accumulator += fn(v[i]);
    return accumulator;
}

int Metrics::calcZeros(const std::vector<int>& v, size_t exclude_idx)
{
    assert(v.size() > 1);

    auto zeros = 0;
    for (int i = 0; i < v.size(); i++)
        if (i != exclude_idx && v[i] == 0)
            zeros++;
    return zeros;
}

int Metrics::calcMin(const std::vector<int>& v, size_t exclude_idx)
{
    assert(v.size() > 1);
    
    auto min_value = INT_MAX;
    for (size_t i = 0; i < v.size(); i++)
        if (i != exclude_idx && v[i] < min_value)
            min_value = v[i];
    return min_value;
}

int Metrics::calcMax(const std::vector<int>& v, size_t exclude_idx)
{
    assert(v.size() > 1);
    
    auto max_value = INT_MIN;
    for (size_t i = 0; i < v.size(); i++)
        if (i != exclude_idx &&  v[i] > max_value)
            max_value = v[i];
    return max_value;
}

double Metrics::calcAverage(const std::vector<int>& v, size_t exclude_idx)
{
    assert(v.size() > 1);
    
    double avg = 0.0;
    for (size_t i = 0; i < v.size(); i++)
    {
        if (i != exclude_idx) {
            avg += v[i];
        }
    }
    
    avg /= (v.size() - 1);
    return avg;
}


double Metrics::calcSquareDeviation(const std::vector<int>& v, size_t exclude_idx)
{
    double avg = calcAverage(v, exclude_idx);
    return calcSquareDeviation(v, exclude_idx, avg);
}

double Metrics::calcSquareDeviation(const std::vector<int>& v, size_t exclude_idx, double target)
{
    size_t count = 0;
    double sd = 0.0;
    for (size_t i = 0; i < v.size(); i++)
    {
        if (i != exclude_idx) {
            sd += (v[i] - target) * (v[i] - target);
            count++;
        }
    }

    auto result = sd / count;
    return result;
}