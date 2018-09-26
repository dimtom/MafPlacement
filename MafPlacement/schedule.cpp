#include "schedule.h"

size_t Schedule::generateRandomRound() const
{
    assert(_config.numRounds() >= 2);

    size_t round = rand() % _config.numRounds();
    if (_rounds[round].games().size() < 2) {
        round--;
        assert(_rounds[round].games().size() < 2);
    }

    return round;
}

void Schedule::generateRandomGames(size_t round, size_t* out_game_one, size_t* out_game_two) const
{
    size_t game_low = round * _config.numTables();
    size_t game_high = (round + 1 < _config.numRounds())
        ? (round + 1) * _config.numTables()
        : _config.numGames();

    size_t games_in_round = game_high - game_low;
    assert(games_in_round >= 2);

    size_t game_shift_one = rand() % games_in_round;
    size_t game_add_two = 1 + (rand() % (games_in_round - 1));
    size_t game_shift_two = (game_shift_one + game_add_two) % games_in_round;
    
    *out_game_one = game_low + game_shift_one;
    *out_game_two = game_low + game_shift_two;
}

seat_t Schedule::generateRandomSeat() const
{
    seat_t seat = static_cast<seat_t>(rand() % Configuration::NumSeats);
    return seat;
}

player_t Schedule::generateRandomPlayer() const
{
    player_t player = static_cast<player_t>(rand() % _config.numPlayers());
    return player;

}

bool Schedule::randomSeatChange(std::function<double()> fn)
{
    auto round = generateRandomRound();
    return randomSeatChange(fn, round);
}

bool Schedule::randomSeatChange(std::function<double()> fn, size_t round)
{
    size_t game_one;
    size_t game_two;
    generateRandomGames(round, &game_one, &game_two);

    return randomSeatChangeInGames(fn, game_one, game_two);
}

bool Schedule::randomSeatChangeInGames(
    std::function<double()> fn,
    size_t game1_idx,
    size_t game2_idx)
{
    assert(game1_idx != game2_idx);

    auto& g1 = _games[game1_idx];
    auto& g2 = _games[game2_idx];
        
    const int MAX_ITERATIONS = 100;
    for (size_t i = 0; i < MAX_ITERATIONS; i++) {
        seat_t pos1 = rand() % Configuration::NumSeats;
        seat_t pos2 = rand() % Configuration::NumSeats;

        // TODO: MUST check it we try to put a player to a game where there is already a player !!!

        // try to swap players
        
        player_t player1 = g1.getPlayerAtSeat(pos1);
        player_t player2 = g2.getPlayerAtSeat(pos2);

        if (canSwitchPlayers(player1, game1_idx, player2, game2_idx))
        {
            double score_before = fn();
            switchPlayers(player1, game1_idx, player2, game2_idx);
            double score_after = fn();
            if (score_after >= score_before) {
                switchPlayers(player2, game1_idx, player1, game2_idx);
                return false;
            }
            return true;
        }
    }

    // cound not found a valid pair
    return false;
}

/*bool Schedule::randomPlayerChange(std::function<double()> fn)
{
    auto round = generateRandomRound();
    return randomPlayerChange(fn, round);
}

bool Schedule::randomPlayerChange(std::function<double()> fn, size_t round)
{
    size_t game_one;
    size_t game_two;
    generateRandomGames(round, &game_one, &game_two);

    return randomPlayerChangeInGames(fn, game_one, game_two);
}

bool Schedule::randomPlayerChangeInGames(
    std::function<double()> fn,
    size_t game1_idx,
    size_t game2_idx)
{
    assert(game1_idx != game2_idx);

    auto& g1 = _games[game1_idx];
    auto& g2 = _games[game2_idx];

    const int MAX_ITERATIONS = 100;
    for (size_t i = 0; i < MAX_ITERATIONS; i++) {
        seat_t pos1 = rand() % Configuration::NumSeats;
        seat_t pos2 = rand() % Configuration::NumSeats;

        // TODO: MUST check it we try to put a player to a game where there is already a player !!!

        // try to swap players

        player_t player1 = g1.getPlayerAtSeat(pos1);
        player_t player2 = g2.getPlayerAtSeat(pos2);

        if (canSwitchPlayers(player1, game1_idx, player2, game2_idx))
        {
            double score_before = fn();
            switchPlayers(player1, game1_idx, player2, game2_idx);
            double score_after = fn();
            if (score_after >= score_before) {
                switchPlayers(player2, game1_idx, player1, game2_idx);
                return false;
            }
            return true;
        }
    }

    // cound not found a valid pair
    return false;
}*/

bool Schedule::canSwitchPlayers(
    player_t player_a, size_t idx_game_a,
    player_t player_b, size_t idx_game_b) const
{
    auto& game_a = _games[idx_game_a];
    auto& game_b = _games[idx_game_b];
    return (game_a.canSubstitutePlayer(player_a, player_b) &&
        game_b.canSubstitutePlayer(player_b, player_a));
}

void Schedule::switchPlayers(
    player_t player_a, size_t idx_game_a,
    player_t player_b, size_t idx_game_b)
{
    auto& game_a = _games[idx_game_a];
    auto& game_b = _games[idx_game_b];

    // TODO: can put into assert
    if (!canSwitchPlayers(player_a, idx_game_a, player_b, idx_game_b)) {
        throw std::exception("can not switch players!");
    }

    game_a.substitutePlayer(player_a, player_b);
    game_b.substitutePlayer(player_b, player_a);
}








