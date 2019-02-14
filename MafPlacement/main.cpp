#include <cstdio>
#include <cstdlib>

#include <algorithm> 
#include <map>
#include <memory>

#include "log.h"
#include "core.h"
#include "metrics.h"
#include "schedule.h"
#include "random_optimizer.h"
#include "seat_optimizer.h"

//
// forward declarations
//

std::unique_ptr<Schedule> solvePlayers(
    const Configuration& conf, 
    size_t num_stages,
    size_t num_iterations);
std::unique_ptr<Schedule> solveSeats(
    const Schedule& schedule,
    size_t num_stages,
    size_t num_iterations);

void outputInitial(const Schedule& schedule);
void outputPlayerOptimization(const Schedule& schedule);
void outputSeatOptimization(const Schedule& schedule);
void outputFinal(const Schedule& schedule);

void printConfiguration(const Configuration& conf);
void printScheduleByGames(const Schedule& schedule);
void printSchedulebyRounds(const Schedule& schedule);
void printScheduleByPlayers(const Schedule& schedule);
void printScheduleByPlayers2(const Schedule& schedule);
void printGame(const Game& game);


void usage();
void verifyParams(int players, int rounds, int tables, int games, int* out_attempts);

// --------------------------------------------------------------------------
// main methods
// --------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc < 3) {
        usage();
        return -1;
    }

    int players = atoi(argv[1]);
    int rounds = atoi(argv[2]);
    int tables = atoi(argv[3]);

    int games = (argc > 4)
        ? atoi(argv[4])
        : tables * rounds;

    // verify params and calculate attempts
    int attempts = 0;
    try {
        verifyParams(players, rounds, tables, games, &attempts);
    }
    catch (std::exception& ex) {
        printf("Failed to verify parameters:\n%s", ex.what());
        return -1;
    }

    // create config
    Configuration conf(players, rounds, tables, games, attempts);
    printConfiguration(conf);

    // optimize player opponents
    // do not optimize seats yet
    printf("\n*** Optimize player opponents\n");
    size_t num_stages_players = 50;
    size_t max_iterations_players = 500 * 1000;
    auto players_schedule = solvePlayers(conf, num_stages_players, max_iterations_players);

    printf("\n*** After optimization\n");
    outputPlayerOptimization(*players_schedule);

    // don't switch players from-to games
    // do only in-game seat optimization
    //printf("\n*** Seats before\n");
    //outputSeatOptimization(*players_schedule);
    
    printf("\n*** Optimize seats\n");
    size_t max_stages_seats = 10;
    size_t max_iterations_seats = 10 * 1000 * 1000;
    auto seats_schedule = solveSeats(*players_schedule, max_stages_seats, max_iterations_seats);
    
    //printf("\n*** Seats after\n");
    //outputSeatOptimization(*seats_schedule);

    // print the best schedule
    printf("\n*** Final schedule\n");
    outputFinal(*seats_schedule);

    return 0;
}

void usage()
{
    printf("Usage: <players> <rounds> <tables> <games>\n");
}

void verifyParams(int players, int rounds, int tables, int games, int* out_attempts)
{
    if (players < 10) {
        throw std::invalid_argument("Number of players must be more than 10");
    }

    if (rounds < 1) {
        throw std::invalid_argument("Number of rounds must be positive!");
    }

    if (tables < 1) {
        throw std::invalid_argument("Number of tables must be positive");
    }

    int low_limit = (rounds - 1) * tables;
    int high_limit = rounds * tables;
    if (games <= low_limit) {
        static char msg[1024];
        sprintf_s(msg, "Number of games is too low: (%d, %d]", low_limit, high_limit);
        throw std::invalid_argument(msg);
    }

    if (games > high_limit) {
        static char msg[1024];
        sprintf_s(msg, "Number of games is too high: (%d, %d]", low_limit, high_limit);
        throw std::invalid_argument(msg);
    }

    // calc attempts
    if ((10 * games) % players != 0) {
        static char msg[1024];
        sprintf_s(msg, "Parameters mismatch. players_every_game(10) * games(%d) / players (%d) should be integer",
            games, players);
        throw std::invalid_argument(msg);
    }
    *out_attempts = 10 * games / players;
}


// --------------------------------------------------------------------------
// solve and optimization methods
// --------------------------------------------------------------------------

double calcPlayerScore(const Schedule& schedule, Metrics& metrics)
{
    const auto& conf = schedule.config();

    double sd_penalty = 0.0;
    double add_penalty = 0.0;
    double target = 9.0 * conf.numAttempts() / (conf.numPlayers() - 1);
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);

        double sd = Metrics::calcSquareDeviation(opponents, player, target);
        sd_penalty += sd;

        //int k[11] = { 100, 50, 0, 0, 0, 0, 20, 100, 200, 400, 800 };
        //int k[11] = { 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        int k[11] = { 500, 100, 10, 0, 10, 50, 150, 200, 400, 800, 1000 };
        add_penalty += metrics.aggregate(opponents, player, [&k](int value) { return k[value]; });
    }

    return sd_penalty + add_penalty;
}

double calcSeatScore(const Schedule& schedule, Metrics& metrics)
{
    const auto& conf = schedule.config();

    double target = conf.numAttempts() / (double)Configuration::NumSeats;
    double sd_penalty = 0.0;
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto seats = metrics.calcPlayerSeatsHistogram(player);

        double sd = Metrics::calcSquareDeviation(seats, -1, target);
        sd_penalty += sd;
    }

    return sd_penalty;
}

std::unique_ptr<Schedule> solvePlayers(const Configuration& conf, 
    size_t num_stages, 
    size_t num_iterations)
{
    // calculate only ONE single initial schedule - just to reduce computations
    // assign this variable to ONE to get as many trivial initial schedules as possible
    player_t player_step = static_cast<player_t>(conf.numPlayers());

    printf("\n *** Player optimization\n");
    printf("player_step: %d\n", player_step);
    printf("Num stages: %zu\n", num_stages);
    printf("Num iterations on every stage: %zu\n", num_iterations);

    double best_score = FLT_MAX;
    double worst_score = FLT_MIN;
    std::unique_ptr<Schedule> best_schedule;
    
    for (player_t player_shift = 0; player_shift < conf.numPlayers(); player_shift += player_step) {
        printf("\n* Player shift: %d\n", player_shift);
        for (size_t stage = 0; stage < num_stages; ++stage) {
            // create initial schedule
            auto schedule = createSchedule(conf, player_shift);

            // print initial schedule
            // outputInitial(*schedule);
            
            Metrics metrics(*schedule);
            RandomOptimizer optimizer(*schedule, num_iterations,
                [&](const Schedule& s) { return calcPlayerScore(s, metrics); });
            double score = optimizer.optimize();
            printf("Stage: %3zu. Score: %10.2f\n", stage, score);

            if (score > worst_score) {
                worst_score = score;
            }
            if (score < best_score) {
                best_score = score;
                best_schedule = std::move(schedule);
            }
        }
    }
    
    printf("Best score: %8.4f\n", best_score);
    printf("Worst score: %8.4f\n", worst_score);

    // return the best schedule
    return std::move(best_schedule);
}

std::unique_ptr<Schedule> solveSeats(
    const Schedule& initial_schedule,
    size_t num_stages,
    size_t num_iterations)
{
    printf("\n *** Seat optimization\n");
    printf("Num stages: %zu\n", num_stages);
    printf("Num iterations on every stage: %zu\n", num_iterations);

    double best_score = FLT_MAX;
    double worst_score = FLT_MIN;
    std::unique_ptr<Schedule> best_schedule;
    
    for (size_t stage = 0; stage < num_stages; stage++) {
        Schedule schedule = initial_schedule;
        Metrics metrics(schedule);
        SeatOptimizer optimizer(schedule, num_iterations,
            [&](const Schedule& s) { return calcSeatScore(s, metrics); });
        double score = optimizer.optimize();
        printf("Stage: %3zu. Score: %10.2f\n", stage, score);

        if (score > worst_score) {
            worst_score = score;
        }
        if (score < best_score) {
            best_score = score;
            best_schedule = std::make_unique<Schedule>(schedule);
        }
    }

    printf("Best score: %8.4f\n", best_score);
    printf("Worst score: %8.4f\n", worst_score);
    return std::move(best_schedule);
}

// --------------------------------------------------------------------------
// print/output methods
// --------------------------------------------------------------------------

void outputPlayerMatrix(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    printf("\nPlayer opponents:\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);

        auto print_player = 1 + player;
        printf("Player %2d: ", print_player);
        for (const auto& v : opponents) {
            printf("%3d", v);
        }
        printf("\n");
    }
}

void outputPlayerStatistics(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    printf("\nPlayer statistics:\n");
    double target = 9.0 * conf.numAttempts() / (conf.numPlayers() - 1);
    printf("Each player should play %2.6f times with one another\n", target);
    printf("            min  max      sd\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);

        auto print_player = 1 + player;
        printf("Player %2d: ", print_player);

        int minv = Metrics::calcMin(opponents, player);
        int maxv = Metrics::calcMax(opponents, player);
        double avg = Metrics::calcAverage(opponents, player);

        double sd1 = Metrics::calcSquareDeviation(opponents, player);
        //double sd2 = Metrics::calcSquareDeviation(opponents, player, target);

        printf("%3d %3d        %2.6f\n", minv, maxv, sd1);
    }
}

void outputPairsHistogram(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // calc pairs histogram
    std::vector<int> pair_histogram(conf.numAttempts() + 1, 0);
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        for (size_t i = 0; i < player; i++) {
            auto num_games_together = opponents[i];
            pair_histogram[num_games_together]++;
        }
    }

    // output pairs histogram
    size_t num_pairs = 0;
    printf("\nPairs histogram:\n");
    for (size_t i = 0; i < pair_histogram.size(); i++)
    {
        auto value = pair_histogram[i];
        num_pairs += value;

        printf("%2zu: %d\n", i, value);
    }
    printf("Total number of pairs: %zu\n", num_pairs);
}

void outputInitial(const Schedule& schedule)
{
    // print initial schedule
    printSchedulebyRounds(schedule);
    printScheduleByPlayers(schedule);

    if (!verifySchedule(schedule)) {
        throw std::exception("schedule is not valid");
    }
}

void outputFinal(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // print schedule
    printSchedulebyRounds(schedule);
    printScheduleByPlayers(schedule);
    printScheduleByPlayers2(schedule);
    if (!verifySchedule(schedule)) {
        throw std::exception("schedule is not valid");
    }

    outputPlayerMatrix(schedule);
    outputPairsHistogram(schedule);
    outputPlayerStatistics(schedule);
    outputSeatOptimization(schedule);
}

void outputPlayerOptimization(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // print schedule
    //printSchedulebyRounds(schedule);
    //printScheduleByPlayers(schedule);
    printScheduleByPlayers2(schedule);
    if (!verifySchedule(schedule)) {
        throw std::exception("schedule is not valid");
    }

    //outputPlayerMatrix(schedule);
    outputPairsHistogram(schedule);
    // outputPlayerStatistics(schedule);
}


void outputSeatOptimization(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    printf("\nPlayer seats:\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto seats = metrics.calcPlayerSeatsHistogram(player);
        
        auto print_player = 1 + player;
        printf("Player %2d: ", print_player);
        for (const auto& v : seats)
            printf("%4d", v);
        printf("\n");
    }
}

void printConfiguration(const Configuration& conf)
{
    printf("\n*** Configuration\n");
    printf("Players: %zu\n", conf.numPlayers());
    printf("Rounds: %zu\n", conf.numRounds());
    printf("Tables per round: %zu\n", conf.numTables());
    printf("Total nunber of games: %zu\n", conf.numGames());
    printf("Number of attempts (games played by each player during tournament): %zu\n", conf.numAttempts());
}

void printScheduleByGames(const Schedule& schedule)
{
    printf("*** Schedule by games\n");

    int game_num = 0;
    for (const auto& game : schedule.games()) {
        game_num++;
        printf("Game %3d >> ", game_num);
        printGame(game);
    }
}

void printSchedulebyRounds(const Schedule& schedule)
{
    printf("*** Schedule by rounds\n");

    int round_num = 0;
    int game_num = 0;
    for (const auto& round : schedule.rounds()) {
        round_num++;
        printf("* Round %2d\n", round_num);
        for (const Game* game : round.games()) {
            game_num++;
            printf("Game %3d >> ", game_num);
            printGame(*game);
        }
        printf("\n");
    }
}

void printScheduleByPlayers(const Schedule& schedule)
{
    printf("*** Schedule for players\n");

    auto& conf = schedule.config();
    for (player_t player = 0; player < conf.numPlayers(); player++) {
        auto print_player = 1 + player;
        printf("* Player %3d: ", print_player);
        
        int round_num = 0;
        for (const auto& round : schedule.rounds()) {
            round_num++;

            int game_num = 0;
            bool found = false;
            for (const Game* game : round.games()) {
                game_num++;
                if (game->participates(player)) {
                    // found a table and a seat for this player this round
                    auto print_seat = 1 + game->players()[player];
                    printf("%2d/%2d ", game_num, print_seat);
                    found = true;
                    break;
                }
            }

            if (!found) {
                // no game for this player this round
                printf(" */*  ");
            }
        }
        printf("\n");
    }
}

void printScheduleByPlayers2(const Schedule& schedule)
{
    printf("*** Schedule for players2\n");

    auto& conf = schedule.config();
    for (player_t player = 0; player < conf.numPlayers(); player++) {
        auto print_player = 1 + player;

        int round_num = 0;
        for (const auto& round : schedule.rounds()) {
            round_num++;

            int game_num = 0;
            bool found = false;
            for (const Game* game : round.games()) {
                if (game->participates(player)) {
                    // found a table and a seat for this player this round
                    auto print_seat = 1 + game->players()[player];
                    printf("%2d ", game_num);
                    found = true;
                    break;
                }
                game_num++;
            }

            if (!found) {
                // no game for this player this round
                printf(" * ");
            }
        }
        printf("\n");
    }
}

void printGame(const Game& game)
{
    for (const auto& s : game.seats()) {
        auto print_seat = 1 + s;
        printf("%3u", print_seat);
    }
    printf("\n");
}






