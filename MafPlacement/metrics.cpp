#include "metrics.h"

// histogram of player seats
std::vector<int> 
Metrics::CalcPlayerSeatsHistogram(int player_id)
{
    std::vector<int> player_seats(Game::NumSeats, 0);
    for (const auto& game : _schedule.games())
    {
        for (size_t i = 0; i < Game::NumSeats; i++) {
            if (game.seats()[i] == player_id)
                player_seats[i]++;
        }
    }

    return player_seats;
}

// histogram of player opponents
std::vector<int>
Metrics::CalcPlayerOpponentsHistogram(int player_id)
{
    std::vector<int> player_opponents(_schedule.configuration().players(), 0);
    for (const auto& game : _schedule.games())
    {
        bool good_game = false;
        for (size_t i = 0; i < Game::NumSeats; i++) {
            if (game.seats()[i] == player_id)
                good_game = true;
        }

        if (!good_game) {
            continue;
        }

        for (size_t i = 0; i < Game::NumSeats; i++) {
            auto id = game.seats()[i];
            player_opponents[id]++;
        }
    }

    return player_opponents;
}