#pragma once
#include <cassert>
#include <vector>

#include "configuration.h"
#include "types.h"

class Game
{
public:
    Game(const Configuration& config, const std::vector<player_t>& seats);
    ~Game() = default;

// 
// construct and initialize
// 
public:
    bool valid() const
    {
        return !_seats.empty() && !_players.empty();
    }

//
// get-set
//
public:
    // returns a map seat -> player_id
    const std::vector<player_t>& seats() const;

    // returns a map: player_id -> their seat in this game
    // or NoSeat this player does not take part in a game
    const std::vector<seat_t>& players() const;

    bool participates(player_t player_id) const;

    // returns player id of given seat index
    player_t getPlayerAtSeat(seat_t seat_idx) const;

    // changes player id of given seat index
    void putPlayerToSeat(seat_t seat_idx, player_t new_player_id);

    bool canSubstitutePlayer(player_t a, player_t b) const;
    
    void substitutePlayer(player_t a, player_t b);

    void switchSeats(size_t seat_one, size_t seat_two);

private:
    const Configuration& _config;
    
    std::vector<player_t> _seats;
    std::vector<seat_t> _players;
};
