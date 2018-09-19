#pragma once

class Configuration
{
public:
    Configuration(int players, int rounds, int tables, int games, int attempts)
        : _players(players)
        , _rounds(rounds)
        , _tables(tables)
        , _games(games)
        , _attempts(attempts)
    {}
    ~Configuration() = default;

public:
    int players() const { return _players; }
    int tables() const { return _tables; }
    int rounds() const { return _rounds;  }
    int games() const { return _games;  }
    int attempts() const { return _attempts; }

private:
    // number of players in the tournament
    int _players;

    // max number of tables in every round
    int _tables;

    // number of rounds in tournament
    int _rounds;

    // total number of games in the tournament
    int _games; 

    // number of games per each player
    int _attempts;
};
