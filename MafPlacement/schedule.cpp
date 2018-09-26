#include "schedule.h"

bool Schedule::randomChange(std::function<double()> fn)
{
    size_t round = rand() % _config.numRounds();
    return randomChange(fn, round);
}

bool Schedule::randomChange(std::function<double()> fn, size_t round)
{
    size_t game_low = round * _config.numTables();
    size_t game_high = (round + 1 < _config.numRounds())
        ? (round + 1) * _config.numTables()
        : _config.numGames();

    size_t games_in_round = game_high - game_low;
    if (games_in_round <= 1)
        return false;

    size_t game_shift_one = rand() % games_in_round;
    size_t game_add_two = 1 + (rand() % (games_in_round - 1));
    size_t game_shift_two = (game_shift_one + game_add_two) % games_in_round;
    size_t game1 = game_low + game_shift_one;
    size_t game2 = game_low + game_shift_two;

    return randomChangeInGames(fn, game1, game2);
}

bool Schedule::randomChangeInGames(std::function<double()> fn,
    size_t game1_idx, size_t game2_idx)
{
    assert(game1_idx != game2_idx);

    auto& g1 = _games[game1_idx];
    auto& g2 = _games[game2_idx];

    seat_t pos1 = rand() % Configuration::NumSeats;
    seat_t pos2 = rand() % Configuration::NumSeats;

    // try to swap players
    double score_before = fn();
    player_t player1 = g1.getPlayerAtSeat(pos1);
    player_t player2 = g2.getPlayerAtSeat(pos2);
    g1.putPlayerToSeat(pos1, player2);
    g2.putPlayerToSeat(pos2, player1);

    double score_after = fn();
    if (score_after >= score_before) {
        // roll back
        g1.putPlayerToSeat(pos1, player1);
        g2.putPlayerToSeat(pos2, player2);
        return false;
    }

    return true;
}