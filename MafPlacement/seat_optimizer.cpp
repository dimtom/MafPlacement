#include "seat_optimizer.h"

#include <algorithm>
#include <cstdlib>

static std::vector<player_t> s_old_seats;
static std::vector<player_t> s_old_seats_one;
static std::vector<player_t> s_old_seats_two;

bool SeatOptimizer::shuffleGame(size_t game_idx)
{
    // shuffle the whole game!
    auto score_before = _score_fn(_schedule);
    s_old_seats = _schedule.games()[game_idx].seats();
    _schedule.games()[game_idx].shuffleSeats();
    auto score_after = _score_fn(_schedule);

    if (score_after >= score_before) {
        // roll back only 2 players
        //_schedule.switchSeats(game_idx, seat_two, seat_one);

        // roll back the whole game
        _schedule.games()[game_idx].setSeats(s_old_seats);
        return false;
    }

    return true;
}

bool SeatOptimizer::shuffleGames(size_t game_idx_one, size_t game_idx_two)
{
    if (game_idx_one == game_idx_two) {
        return false;
    }

    // shuffle the whole game!
    auto score_before = _score_fn(_schedule);
    
    s_old_seats_one = _schedule.games()[game_idx_one].seats();
    s_old_seats_two = _schedule.games()[game_idx_two].seats();
    _schedule.games()[game_idx_one].shuffleSeats();
    _schedule.games()[game_idx_two].shuffleSeats();
    auto score_after = _score_fn(_schedule);

    if (score_after >= score_before) {
        // roll back the whole game
        _schedule.games()[game_idx_one].setSeats(s_old_seats_one);
        _schedule.games()[game_idx_two].setSeats(s_old_seats_two);
        return false;
    }

    return true;

}

bool SeatOptimizer::switchTwoSeats(size_t game_idx)
{
    Game& game = _schedule.games()[game_idx];

    size_t seat_one = _schedule.generateRandomSeat();
    size_t seat_two = _schedule.generateRandomSeat();
    while (seat_two == seat_one) {
        seat_two = _schedule.generateRandomSeat();
    }

    // switch only 2 players
    auto score_before = _score_fn(_schedule);
    _schedule.switchSeats(game, seat_one, seat_two);
    auto score_after = _score_fn(_schedule);

    if (score_after >= score_before) {
        // roll back only 2 players
        _schedule.switchSeats(game, seat_two, seat_one);
        return false;
    }

    return true;
}

double SeatOptimizer::optimize()
{    
    for (size_t stage = 0; stage < _max_stages; stage++) 
    {
        // size_t game_idx = _schedule.generateRandomGame();

        size_t good_shuffle_iterations = 0;
        size_t good_switch_iterations = 0;

        // shuffling the whole game
        for (size_t i = 0; i < _shuffle_per_stage; i++)
        {
            size_t game_idx = _schedule.generateRandomGame();
            good_shuffle_iterations += shuffleGame(game_idx);
        }

        // switching only 2 seats in the game
        for (size_t i = 0; i < _switch_per_stage; i++)
        {
            size_t game_idx = _schedule.generateRandomGame();
            good_switch_iterations += switchTwoSeats(game_idx);
        }

        // shuffling 2 whole games
        for (size_t i = 0; i < _shuffle_per_stage; i++)
        {
            size_t game_idx_one = _schedule.generateRandomGame();
            size_t game_idx_two = _schedule.generateRandomGame();
            good_shuffle_iterations += shuffleGames(game_idx_one, game_idx_two);
        }

        auto score = _score_fn(_schedule);
        printf("Stage #%zu: score=%6.2f good shuffle iterations: %zu good switch iterations: %zu\n", 
            stage, score, good_shuffle_iterations, good_switch_iterations);
    }

    auto score = _score_fn(_schedule);
    return score;
}