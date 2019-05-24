#pragma once
#include <cassert>
#include <vector>

#include "configuration.h"
#include "types.h"

//
// class Game - represents who plays in mafia game
// and what seats each player holds
//
class Game
{
public:
    Game(const Configuration& config, const std::vector<player_t>& seats);
    ~Game() = default;

public:
    bool valid() const
    {
        return !_seats.empty() && !_players.empty();
    }

public:
    // returns a map seat -> player_id
    // size of array: 10 (Configuration::NumSeats)
    const std::vector<player_t>& seats() const;

    // returns a map: player_id -> their seat in this game
    // or NoSeat this player does not take part in a game
    // size of array: Configuration::NumPlayers
    const std::vector<seat_t>& players() const;

    // returns if player_id participates in the game
    bool participates(player_t player_id) const;

    // returns player id of given seat index
    player_t getPlayerAtSeat(seat_t seat_idx) const;

    // changes player id of given seat index
    void putPlayerToSeat(seat_t seat_idx, player_t new_player_id);

    bool canSubstitutePlayer(player_t a, player_t b) const;
    
    void substitutePlayer(player_t a, player_t b);

    void switchSeats(size_t seat_one, size_t seat_two);

    void setSeats(const std::vector<player_t>& seats);
    void shuffleSeats();

private:
    void calcPlayers();

private:
    const Configuration& _config;
    
    std::vector<player_t> _seats;
    std::vector<seat_t> _players;
};
