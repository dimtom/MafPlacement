#include "game.h"
#include <algorithm> 
#include <cassert>
#include <stdexcept>
#include <random>

Game::Game(const Configuration& config, const std::vector<player_t>& seats)
    : _config(config)
{
    // sanity check
    if (seats.size() != Configuration::NumSeats) {
        char msg[4096];
        sprintf(msg, "Can not create a game, expected number of seats %zu, got %zu instead.",
            Configuration::NumSeats, seats.size());
        throw std::invalid_argument(msg);
    }

    // populate seats
    _seats = std::move(seats);

    calcPlayers();
}

void Game::calcPlayers()
{
    assert(_seats.size() == Configuration::NumSeats);

    // populate players' seats
    _players.resize(_config.numPlayers(), InvalidSeatId);
    for (size_t idx = 0; idx < _seats.size(); idx++) {
        auto player_id = _seats[idx];

        assert(player_id >= 0 && player_id < _config.numPlayers());
        // assert(_players[player_id] == InvalidSeatId);
        _players[player_id] = static_cast<seat_t>(idx);
    }
}

void Game::setSeats(const std::vector<player_t>& seats)
{
    assert(seats.size() == Configuration::NumSeats);
    _seats = std::move(seats);
    calcPlayers();
}

void Game::shuffleSeats()
{
    std::shuffle(_seats.begin(), _seats.end(), std::default_random_engine(0));
    calcPlayers();
}

// returns a map seat -> player_id
const std::vector<player_t>& Game::seats() const
{
    return _seats;
}

// returns a map: player_id -> their seat in this game
// or NoSeat this player does not take part in a game
const std::vector<seat_t>& Game::players() const
{
    return _players;
}

bool Game::participates(player_t player_id) const
{
    return _players[player_id] != InvalidSeatId;
}

// returns player id of given seat index
player_t Game::getPlayerAtSeat(seat_t seat_idx) const
{
    return _seats[seat_idx];
}

// changes player id of given seat index
void Game::putPlayerToSeat(seat_t seat_idx, player_t new_player_id)
{
    assert(new_player_id >= 0 && new_player_id < _config.numPlayers());

    auto old_player_id = _seats[seat_idx];
    _seats[seat_idx] = new_player_id;

    _players[old_player_id] = InvalidSeatId;
    _players[new_player_id] = seat_idx;
}

bool Game::canSubstitutePlayer(player_t a, player_t b) const
{
    return _players[a] != InvalidSeatId && _players[b] == InvalidSeatId;
}

void Game::substitutePlayer(player_t a, player_t b)
{
    auto seat = _players[a];
    assert(canSubstitutePlayer(a, b));

    assert(_seats[seat] == a);
    _seats[seat] = b;

    _players[a] = InvalidSeatId;
    _players[b] = seat;
}

void Game::switchSeats(size_t seat_one, size_t seat_two)
{
    assert(seat_one < Configuration::NumSeats);
    assert(seat_two < Configuration::NumSeats);

    auto player_one = _seats[seat_one];
    auto player_two = _seats[seat_two];

    std::swap(_seats[seat_one], _seats[seat_two]);
    std::swap(_players[player_one], _players[player_two]);
}