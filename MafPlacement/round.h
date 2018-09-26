#pragma once
#include <vector>

class Game;

class Round
{
public:
    Round(std::vector<Game*> games)
        : _games(games)
    {}

    ~Round() = default;

public:
    const std::vector<Game*> games() const
    {
        return _games;
    }
    
private:
    std::vector<Game*> _games;

};
