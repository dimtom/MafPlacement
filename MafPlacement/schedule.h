#pragma once

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

private:
    const Configuration& _configuration;
    std::vector<Game> _games;


};
