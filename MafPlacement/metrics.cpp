#include "metrics.h"
#include <cassert>

// histogram of player seats
std::vector<int> 
Metrics::calcPlayerSeatsHistogram(int player_id)
{
    std::vector<int> player_seats(Configuration::NumSeats, 0);
    for (const auto& game : _schedule.games())
    {
        for (size_t i = 0; i < Configuration::NumSeats; i++) {
            if (game.seats()[i] == player_id)
                player_seats[i]++;
        }
    }

    return player_seats;
}

// histogram of player opponents
std::vector<int>
Metrics::calcPlayerOpponentsHistogram(int player_id)
{
    std::vector<int> player_opponents(_schedule.config().numPlayers(), 0);
    for (const auto& game : _schedule.games())
    {
        bool good_game = false;
        for (size_t i = 0; i < Configuration::NumSeats; i++) {
            if (game.seats()[i] == player_id)
                good_game = true;
        }

        if (!good_game) {
            continue;
        }

        for (size_t i = 0; i < Configuration::NumSeats; i++) {
            auto id = game.seats()[i];
            player_opponents[id]++;
        }
    }

    player_opponents[player_id] = 0; // exclude yourself vs yourself
    return player_opponents;
}

int Metrics::calcMin(const std::vector<int>& v)
{
    assert(!v.empty());
    
    auto min_value = v[0];
    for (int i = 1; i < v.size(); i++)
        if (v[i] < min_value)
            min_value = v[i];
    return min_value;
}

int Metrics::calcMax(const std::vector<int>& v)
{
    assert(!v.empty());
    
    auto max_value = v[0];
    for (int i = 1; i < v.size(); i++)
        if (v[i] > max_value)
            max_value = v[i];
    return max_value;
}

double Metrics::calcAverage(const std::vector<int>& v)
{
    assert(!v.empty());
    
    double avg = 0.0;
    for (auto value : v)
        avg += value;
    
    avg /= v.size();
    return avg;
}


double Metrics::calcSquareDeviation(const std::vector<int>& v)
{
    double avg = calcAverage(v);
    return calcSquareDeviation(v, avg);
}

double Metrics::calcSquareDeviation(const std::vector<int>& v, double target)
{
    double sd = 0.0;
    for (int i = 0; i < v.size(); i++)
    {
        sd += (v[i] - target) * (v[i] - target);
    }

    sd /= v.size();
    return sd;
}