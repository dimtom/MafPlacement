#include "schedule.h"

#include <cassert>
#include <memory>
#include <set>
#include <stdexcept>

std::unique_ptr<Schedule>
Schedule::createInitialSchedule(const Configuration& conf, player_t shift_player_num = 0)
{
    // we need to create a schedule so that every player 
    // take part in equal number of games
    std::vector<int> num_games_played(conf.numPlayers(), static_cast<int>(conf.numAttempts()));

    int game_num = 0;
    player_t player_num = shift_player_num % conf.numPlayers();
    std::vector<Game> games;
    for (int r = 0; r < conf.numRounds(); r++) {
        for (int t = 0; t < conf.numTables(); t++) {
            if (game_num++ >= conf.numGames())
                break;

            // create and initialize a game
            std::vector<player_t> seats(conf.NumSeats, InvalidPlayerId);
            for (size_t i = 0; i < seats.size(); i++) {
                seats[i] = player_num;
                num_games_played[player_num]--;
                player_num = ++player_num % conf.numPlayers();
            }

            games.emplace_back(conf, seats);
        }
    }

    auto schedule = std::make_unique<Schedule>(conf, games);
    return schedule;
}

std::unique_ptr<Schedule>
Schedule::createCustomScheduleFromGames(const Configuration& conf, const std::vector<std::vector<player_t>>& seats)
{
    // TODO: here we do not check for seats - range of players, rounds, etc
    std::vector<Game> games;
    assert(seats.size() == conf.numGames());
    for (const auto& s : seats) {
        assert(s.size() == conf.NumSeats);

        // 1-based player id
        /*auto t = s;
        for (auto& player : t)
            player--;
        games.emplace_back(conf, t);*/

        // zero-based player id
        games.emplace_back(conf, s);
    }

    auto schedule = std::make_unique<Schedule>(conf, games);
    return schedule;
}

std::unique_ptr<Schedule>
Schedule::createCustomScheduleFromPlayers(const Configuration& conf, const std::vector<std::vector<int>>& players)
{
    std::vector<std::vector<player_t>> seats;
    seats.resize(conf.numGames());
    for (player_t player_id = 0; player_id < players.size(); player_id++)
    {
        const auto& player_tables = players[player_id];
        assert(player_tables.size() == conf.numRounds());

        size_t round_first_game_id = 0;
        for (auto table_id : player_tables)
        {
            if (table_id != InvalidTableId)
            {
                auto game_id = round_first_game_id + table_id;
                seats[game_id].emplace_back(player_id);
            }
            round_first_game_id += conf.numTables();
        }
    }

    return createCustomScheduleFromGames(conf, seats);
}

bool Schedule::verify() const
{
    // calc number of games played by every player
    std::vector<int> games_played(_config.numPlayers());
    for (const auto& game : _games)
    {
        for (auto id : game.seats())
        {
            games_played[id]++;
        }
    }

    bool ok = true;
    int gp = games_played[0];
    for (size_t i = 1; i < games_played.size(); i++)
    {
        if (games_played[i] != gp) {
            ok = false;
            break;
        }
    }

    // debug output only if we have problems
    if (!ok) {
        printf("Sanity check: \n");
        for (size_t i = 0; i < games_played.size(); i++) {
            printf("%3d", games_played[i]);
        }
        printf("\n");
    }

    return ok;
}

Schedule::Schedule(const Configuration& config, const std::vector<Game>& games)
    : _config(config)
    , _games(std::move(games))
{
    // TODO: sanity check - provided config should be the same as all the games' config!

    if (_games.size() != _config.numGames()) {
        char msg[4096];
        sprintf(msg, "Can not create a schedule, expected number of games: %zu, got %zu instead.",
            _config.numGames(), _games.size());
        throw std::invalid_argument(msg);
    }

    populateRounds();
}

Schedule::Schedule(const Schedule& source)
    : _config(source._config)
    , _games(source._games)
{
    populateRounds();
}

void Schedule::populateRounds()
{
    // populate rounds
    _rounds.clear();
    int game_num = 0;
    for (size_t round = 0; round < _config.numRounds(); round++) {
        std::vector<Game*> games_in_round;
        for (size_t table = 0; table < _config.numTables(); table++) {
            if (game_num < _config.numGames()) {
                games_in_round.push_back(&_games[game_num]);
                game_num++;
            }
        }
        Round r(games_in_round);
        _rounds.push_back(std::move(r));
    }
}

size_t Schedule::generateRandomRound() const {
    assert(_config.numRounds() >= 2);
    size_t round = rand() % _config.numRounds();

    // don't return non-full (last) round
    if (_rounds[round].games().size() < _config.numTables()) {
        assert(round > 0);
        round--;
    }

    // return only FULL round
    assert(_rounds[round].games().size() == _config.numTables());
    return round;
}

size_t Schedule::generateRandomGame() const {
    assert(_config.numGames() >= 2);

    size_t game = rand() % _config.numGames();
    return game;
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
    // special case - only one table
    // switch players in 2 random rounds
    if (_config.numTables() == 1) {
        auto round_one = generateRandomRound();
        auto round_two = generateRandomRound();
        if (round_one == round_two) {
            round_two = (round_two + 1) % _config.numRounds();
        }
        return randomSeatChangeInRouds(fn, round_one, round_two);
    }

    // generic case
    // switch players in 2 games of the same round
    auto round = generateRandomRound();
        
    size_t game_one;
    size_t game_two;
    generateRandomGames(round, &game_one, &game_two);
    return randomSeatChangeInGames(fn, game_one, game_two);
}

// returns (busy) - set of players that are playing the game
// and (free) - set of players that are NOT player the game
static void getParticipantsOfGame(const Game& game,
    std::set<player_t>& busy,
    std::set<player_t>& free) {
    assert(busy.empty());
    assert(free.empty());
    
    const auto& players = game.players();
    for (seat_t idx = 0; idx < players.size(); idx++) {
        seat_t player_seat = players[idx];
        if (player_seat != InvalidSeatId) {
            busy.insert(idx);
        } else {
            free.insert(idx);
        }
    }
}

bool Schedule::randomSeatChangeInRouds(
    std::function<double()> fn,
    size_t round_one, 
    size_t round_two) {
    assert(round_one != round_two);

    // take two games (in different rounds)
    auto game_one_idx = round_one;
    auto game_two_idx = round_two;
    auto& game_one = _games[game_one_idx];
    auto& game_two = _games[game_two_idx];

    std::set<player_t> busy_one;
    std::set<player_t> free_one;
    getParticipantsOfGame(game_one, busy_one, free_one);
    
    std::set<player_t> busy_two;
    std::set<player_t> free_two;
    getParticipantsOfGame(game_two, busy_two, free_two);

    // intersect "plays_one" x "free_two"
    std::vector<player_t> pool_a;
    std::set_intersection(
        busy_one.begin(), busy_one.end(),
        free_two.begin(),free_two.end(), 
        std::back_inserter(pool_a));

    // intersect "plays_two" x "free_one"
    std::vector<player_t> pool_b;
    std::set_intersection(
        busy_two.begin(), busy_two.end(),
        free_one.begin(),free_one.end(), 
        std::back_inserter(pool_b));

    if (pool_a.size() == 0 || pool_b.size() == 0) {
        return false;
        // ### throw std::runtime_error("randomSeatChangeInRouds() - empty pool!");
    }

    // pick 2 random players from corresponsing groups
    size_t player_a_idx = rand() % pool_a.size();
    size_t player_b_idx = rand() % pool_b.size();
    player_t player_a = pool_a[player_a_idx];
    player_t player_b = pool_b[player_b_idx];

    double score_before = fn();
    switchPlayers(game_one, player_a, game_two, player_b);
    double score_after = fn();
    if (score_after >= score_before) {
        switchPlayers(game_one, player_b, game_two, player_a);
        return false;
    }

    if (!verify()) {
        printf("Schedule verification failed. Games: %zu, %zu. Players: %d, %d\n", 
            game_one_idx, game_two_idx,
            player_a, player_b);
        throw std::runtime_error("Schedule verification failed");
    }

    return true;
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

        if (canSwitchPlayers(g1, player1, g2, player2))
        {
            double score_before = fn();
            switchPlayers(g1, player1, g2, player2);
            double score_after = fn();
            if (score_after >= score_before) {
                switchPlayers(g1, player2, g2, player1);
                return false;
            }
            return true;
        }
    }

    // cound not found a valid pair
    return false;
}

bool Schedule::canSwitchPlayers(
    Game& game_a, player_t player_a,
    Game& game_b, player_t player_b) const
{
    return (game_a.canSubstitutePlayer(player_a, player_b) &&
        game_b.canSubstitutePlayer(player_b, player_a));
}

void Schedule::switchPlayers(
    Game& game_a, player_t player_a,
    Game& game_b, player_t player_b)
{
    // TODO: can put into assert
    if (!canSwitchPlayers(game_a, player_a, game_b, player_b)) {
        throw std::runtime_error("can not switch players!");
    }

    game_a.substitutePlayer(player_a, player_b);
    game_b.substitutePlayer(player_b, player_a);
}

void Schedule::switchSeats(Game& game, size_t seat_one, size_t seat_two)
{
    game.switchSeats(seat_one, seat_two);
}

