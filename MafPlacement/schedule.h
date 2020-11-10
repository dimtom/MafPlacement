#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "configuration.h"
#include "game.h"
#include "round.h"
#include "types.h"

// 
// class Schedule - represents a schedule of games.
// Uses Configuration to describe a tournament (number of players, number of rounds, number of games etc).
// Stores Games in a flat vector and grouped by Rounds.
// Provides methods to modify current schedule. 
//
class Schedule
{
public:
    // creates and returns initial schedule for provided parameters (configuration)
    static std::unique_ptr<Schedule>
    createInitialSchedule(const Configuration& conf, player_t shift_player_num);

    // create custom schedule using <games> array
    // games - array of size number of games
    // each item of <games> is array of length(10) - what are the players of this game
    static std::unique_ptr<Schedule>
    createCustomScheduleFromGames(const Configuration& conf, const std::vector<std::vector<player_t>>& games);

    // create custom schedule using <players> array
    // players - array of size - number of players
    // each item of <player>  is array of length <games> - what are the tables the player goes to
    // -1: no game
    static std::unique_ptr<Schedule>
    createCustomScheduleFromPlayers(const Configuration& conf, const std::vector<std::vector<int>>& players);
public:
    Schedule(const Configuration& config, const std::vector<Game>& games);
    Schedule(const Schedule& source);
    ~Schedule() = default;

public:
    bool verify() const;

public:
    const Configuration& config() const {
        return _config;
    }

    const std::vector<Round>& rounds() const {
        return _rounds;
    }

    const std::vector<Game>& games() const {
        return _games;
    }

    // only for shuffle game
    std::vector<Game>& games() {
        return _games;
    }

public:
    // for players optimizer
    bool randomSeatChange(std::function<double()> fn);

    // for seats optimizer
    size_t generateRandomGame() const;
    seat_t generateRandomSeat() const;
    void switchSeats(Game& game, size_t seat_one, size_t seat_two);
    
private:
    size_t generateRandomRound() const;

    // for schedules with >1 tables
    bool randomSeatChangeInGames(std::function<double()> fn,
        size_t game1_idx, size_t game2_idx);

    // for schedules with ONLY ONE table
    bool randomSeatChangeInRouds(std::function<double()> fn, 
        size_t round_one, size_t round_two);

    bool canSwitchPlayers(
        Game& game_a, player_t player_a,
        Game& game_b, player_t player_b) const;

    void switchPlayers(
        Game& game_a, player_t player_a,
        Game& game_b, player_t player_b);

    
    player_t generateRandomPlayer() const;

    void populateRounds();
    void generateRandomGames(size_t round, size_t* out_game_one, size_t* out_game_two) const;


private:
    const Configuration& _config;
    std::vector<Round> _rounds;
    std::vector<Game> _games;

};
