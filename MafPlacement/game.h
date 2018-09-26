#pragma once
#include <cassert>
#include <vector>

#include "types.h"

class Game
{
public:
    Game(const Configuration& config)
        : _config(config)
    {}

    Game(const Configuration& config, const std::vector<player_t> seats)
        : _config(config)
    {
        init(seats);
    }
    
    ~Game() = default;

// 
// construct and initialize
// 
public:
    bool valid() const
    {
        return !_seats.empty() && !_players.empty();
    }

    void init(const std::vector<player_t>& seats)
    {
        _seats.clear();
        _players.clear();

        // initialize seats
        assert(seats.size() == Configuration::NumSeats);
        _seats = std::move(seats);

        // initialize participants
        _players.resize(_config.numPlayers(), InvalidSeatId);
        for (size_t idx = 0; idx < _seats.size(); idx++) {
            auto player_id = seats[idx];
            
            assert(player_id >= 0 && player_id < _config.numPlayers());
            assert(_players[player_id] == InvalidSeatId);
            _players[player_id] = static_cast<seat_t>(idx);
        }
    }

//
// get-set
//
public:
    // returns a map seat -> player_id
    const std::vector<player_t>& seats() const
    {
        return _seats;
    }

    // returns a map: player_id -> their seat in this game
    // or NoSeat this player does not take part in a game
    const std::vector<seat_t>& players() const
    {
        return _players;
    }

    bool participates(player_t player_id) const
    {
        return _players[player_id] != InvalidSeatId;
    }

    // returns player id of given seat index
    player_t getPlayerAtSeat(seat_t seat_idx) const
    {
        return _seats[seat_idx];
    }

    // changes player id of given seat index
    void putPlayerToSeat(seat_t seat_idx, player_t new_player_id)
    {
        assert(new_player_id >= 0 && new_player_id < _config.numPlayers());

        auto old_player_id = _seats[seat_idx];
        _seats[seat_idx] = new_player_id;

        _players[old_player_id] = InvalidSeatId;
        _players[new_player_id] = seat_idx;

    }

private:
    const Configuration& _config;
    
    std::vector<player_t> _seats;
    std::vector<seat_t> _players;
};
