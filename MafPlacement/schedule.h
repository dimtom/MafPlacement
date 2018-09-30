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
    Schedule(const Configuration& config, const std::vector<Game>& games);
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
    bool randomSeatChange(std::function<double()> fn);
    bool randomSeatChange(std::function<double()> fn, size_t round);
    bool randomSeatChangeInGames(std::function<double()> fn,
        size_t game1_idx, size_t game2_idx);

    /*bool randomPlayerChange(std::function<double()> fn);
    bool randomPlayerChange(std::function<double()> fn, size_t round);
    bool randomPlayerChangeInGames(std::function<double()> fn,
        size_t game1_idx, size_t game2_idx);*/


    bool canSwitchPlayers(
        player_t player_a, size_t idx_game_a,
        player_t player_b, size_t idx_game_b) const;

    void switchPlayers(
        player_t player_a, size_t idx_game_a,
        player_t player_b, size_t idx_game_b);

    void switchSeats(size_t game_num, size_t seat_one, size_t seat_two);



public:
    // helper methods for optimizers
    size_t generateRandomRound() const;
    size_t generateRandomGame() const;
    seat_t generateRandomSeat() const;
    player_t generateRandomPlayer() const;

private:
    void generateRandomGames(size_t round, size_t* out_game_one, size_t* out_game_two) const;

    

private:
    const Configuration& _config;
    std::vector<Round> _rounds;
    std::vector<Game> _games;

};
