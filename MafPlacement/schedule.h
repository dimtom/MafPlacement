#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "configuration.h"
#include "game.h"
#include "types.h"

class Schedule
{
public:
    Schedule(const Configuration& config)
        : _config(config)
    {}

    ~Schedule() = default;

public:
    const Configuration& config() const
    {
        return _config;
    }

    void clear()
    {
        _games.clear();
    }

    void addGame(const Game& game)
    {
        _games.emplace_back(std::move(game));
    }

    const std::vector<Game>& games() const
    {
        return _games;
    }

public:
    bool randomChange(std::function<double()> fn)
    {
        int round = rand() % _config.numRounds();
        return randomChange(fn, round);
    }

    bool randomChange(std::function<double()> fn, int round)
    {
        int game_low = round * _config.numTables();
        int game_high = (round + 1 < _config.numRounds())
            ? (round + 1) * _config.numTables()
            : _config.numGames();
        
        int games_in_round = game_high - game_low;
        if (games_in_round <= 1)
            return false;
        
        int game_shift_one = rand() % games_in_round;
        int game_add_two = 1 + (rand() % (games_in_round - 1));
        int game_shift_two = (game_shift_one + game_add_two) % games_in_round;
        int game1 = game_low + game_shift_one;
        int game2 = game_low + game_shift_two;

        return randomChangeInGames(fn, game1, game2);
    }

    bool randomChangeInGames(std::function<double()> fn, 
        int game1_idx, int game2_idx)
    {
        assert(game1_idx != game2_idx);

        auto& g1 = _games[game1_idx];
        auto& g2 = _games[game2_idx];

        seat_t pos1 = rand() % Configuration::NumSeats;
        seat_t pos2 = rand() % Configuration::NumSeats;

        // try to swap players
        double score_before = fn();
        player_t player1 = g1.getPlayerAtSeat(pos1);
        player_t player2 = g2.getPlayerAtSeat(pos2);
        g1.putPlayerToSeat(pos1, player2);
        g2.putPlayerToSeat(pos2, player1);

        double score_after = fn();
        if (score_after >= score_before) {
            // roll back
            g1.putPlayerToSeat(pos1, player1);
            g2.putPlayerToSeat(pos2, player2);
            return false;
        }

        return true;
    }

private:
    const Configuration& _config;
    std::vector<Game> _games;


};
