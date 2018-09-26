#pragma once

class Configuration
{
public:
    // number of players in every game
    static const size_t NumSeats = 10;

public:
    Configuration(int players, int rounds, int tables, int games, int attempts)
        : _numPlayers(players)
        , _numRounds(rounds)
        , _numTables(tables)
        , _numGames(games)
        , _numAttempts(attempts)
    {
        // empty
    }

    ~Configuration() = default;

public:
    // number of players in the tournament
    int numPlayers() const 
    { 
        return _numPlayers; 
    }
    
    // max number of tables in every round
    int numTables() const 
    { 
        return _numTables; 
    }

    // number of rounds in tournament
    int numRounds() const 
    { 
        return _numRounds;  
    }

    // total number of games in the tournament
    int numGames() const 
    { 
        return _numGames;  
    }

    // number of games per each player
    int numAttempts() const 
    { 
        return _numAttempts; 
    }

private:
    int _numPlayers;
    int _numTables;
    int _numRounds;
    int _numGames; 
    int _numAttempts;
};
