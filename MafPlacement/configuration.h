#pragma once

class Configuration
{
public:
    // number of players in every game
    static const size_t NumSeats = 10;

public:
    Configuration(size_t players, size_t rounds, size_t tables, size_t games, size_t attempts)
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
    size_t numPlayers() const
    { 
        return _numPlayers; 
    }
    
    // max number of tables in every round
    size_t numTables() const
    { 
        return _numTables; 
    }

    // number of rounds in tournament
    size_t numRounds() const
    { 
        return _numRounds;  
    }

    // total number of games in the tournament
    size_t numGames() const 
    { 
        return _numGames;  
    }

    // number of games per each player
    size_t numAttempts() const
    { 
        return _numAttempts; 
    }

private:
    size_t _numPlayers;
    size_t _numTables;
    size_t _numRounds;
    size_t _numGames;
    size_t _numAttempts;
};
