#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "configuration.h"
#include "game.h"

class Schedule
{
public:
    Schedule(const Configuration& conf)
        : _configuration(conf)
    {}

    ~Schedule() = default;

public:
    const Configuration& configuration() const
    {
        return _configuration;
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

    bool randomChange(std::function<double()> fn)
    {
        int round = rand() % _configuration.rounds();
        return randomChange(fn, round);
    }

    bool randomChange(std::function<double()> fn, int round)
    {
        int game_low = round * _configuration.tables();
        int game_high = (round + 1 < _configuration.rounds())
            ? (round + 1) * _configuration.tables()
            : _configuration.games();
        
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

        int idx1 = rand() % Game::NumSeats;
        int idx2 = rand() % Game::NumSeats;

        // try to swap players
        double score_before = fn();
        int seat1 = g1.getSeat(idx1);
        int seat2 = g2.getSeat(idx2);
        g1.changeSeat(idx1, seat2);
        g2.changeSeat(idx2, seat1);

        double score_after = fn();
        if (score_after >= score_before) {
            // roll back
            g1.changeSeat(idx1, seat1);
            g2.changeSeat(idx2, seat2);
            return false;
        }

        return true;
    }

private:
    const Configuration& _configuration;
    std::vector<Game> _games;


};
