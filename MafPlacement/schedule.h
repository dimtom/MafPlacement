#pragma once

#include <cassert>
#include <functional>
#include <vector>

#include "configuration.h"
#include "game.h"
#include "round.h"
#include "types.h"

class Schedule
{
public:
    Schedule(const Configuration& config, const std::vector<Game>& games)
        : _config(config)
        , _games(std::move(games))
    {
        // TODO: sanity check - provided config should be the same as all the games' config!

        if (_games.size() != _config.numGames()) {
            char msg[4096];
            sprintf_s(msg, "Can not create a schedule, expected number of games: %zu, got %zu instead.",
                _config.numGames(), _games.size());
            throw std::invalid_argument(msg);
        }

        // populate rounds
        _rounds.clear();
        int game_num = 0;
        for (size_t round = 0; round < _config.numRounds(); round++) {
            std::vector<Game*> games_in_round;
            for (size_t table = 0; table < _config.numTables(); table++) {
                if (game_num < _config.numGames()) {
                    games_in_round.push_back(&_games[game_num]);
                    game_num++;
                }
            }
            Round r(games_in_round);
            _rounds.push_back(std::move(r));
        }
    }

    ~Schedule() = default;

public:
    bool valid() const
    {
        return _games.size() == _config.numGames() && _rounds.size() == _config.numRounds();
    }

public:
    const Configuration& config() const
    {
        return _config;
    }

    const std::vector<Round>& rounds() const
    {
        return _rounds;
    }

    const std::vector<Game>& games() const
    {
        return _games;
    }

public:
    bool randomChange(std::function<double()> fn);
    bool randomChange(std::function<double()> fn, size_t round);
    bool randomChangeInGames(std::function<double()> fn,
        size_t game1_idx, size_t game2_idx);

private:
    const Configuration& _config;
    std::vector<Round> _rounds;
    std::vector<Game> _games;

};
