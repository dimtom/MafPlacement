#pragma once
#include <vector>

class Game;

//
// class Round - represents a set of games played simultineously.
// Therefore, each player can take part not more than in a single
// game of given round.
//
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
