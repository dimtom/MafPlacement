#pragma once

class Configuration
{
public:
    Configuration(int players, int tables, int rounds, int games)
        : _players(players)
        , _tables(tables)
        , _rounds(rounds)
        , _games(games)
    {
        // TODO:
        // Check that current configuration does make sense
    }
    ~Configuration() = default;

public:
    int players() const { return _players; }
    int tables() const { return _tables; }
    int rounds() const { return _rounds;  }
    int games() const { return _games;  }

private:
    // number of players in the tournament
    int _players;

    // max number of tables in every round
    int _tables;

    // number of rounds in tournament
    int _rounds;

    // number of games per each player
    int _games; 
};
