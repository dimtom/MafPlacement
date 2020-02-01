#include <cstdio>
#include <cstdlib>
#include <cfloat>

#include <algorithm> 
#include <map>
#include <memory>
#include <stdexcept>

#include "log.h"
#include "schedule.h"
#include "print.h"
#include "solve.h"

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

    bool optimize_players = (argc > 5)
        ? strcmp(argv[5], "yes") == 0
        : true;

    std::unique_ptr<Schedule> players_schedule;
    if (optimize_players)
    {
        // optimize player opponents
        // do not optimize seats yet
        printf("\n*** Optimize player opponents\n");
        player_t player_step = 11;
        size_t num_stages_players = 5;
        size_t max_iterations_players = 100 * 100 * 100;
        double best_score = DBL_MAX;
        auto lambda = [&best_score](const Schedule& schedule, double score)
        {
            if (score < best_score) {
                best_score = score;
                printf("\n*** Best schedule at the moment, score: %8.4f\n", score);
                outputShortPlayerOptimization(schedule);
            }
            
            return true;
        };
        
        players_schedule = solvePlayers(
            conf,
            player_step,
            num_stages_players,
            max_iterations_players,
            lambda
        );
    }
    else
    {
        printf("*** Loading custom schedule...\n");
        std::vector<std::vector<int>> player_tables =
        {
            {  4,  3,  3,  2,  4,  3,  3,  1,  4,  2},
            {  1,  3,  2,  1,  2,  4,  1,  4,  3,  4},
            {  1,  1,  0,  1,  3,  2,  4,  3,  4,  0},
            {  3,  4,  2,  1,  2,  2,  3,  1,  4,  4},
            {  0,  4,  0,  4,  4,  2,  2,  4,  4,  2},
            {  1,  3,  4,  3,  4,  1,  3,  3,  1,  4},
            {  2,  2,  2,  4,  3,  0,  1,  0,  1,  2},
            {  0,  2,  4,  2,  0,  4,  4,  0,  4,  3},
            {  4,  3,  4,  4,  2,  0,  2,  0,  4,  1},
            {  4,  4,  3,  3,  1,  3,  4,  0,  3,  3},
            {  4,  0,  1,  1,  4,  4,  0,  2,  0,  2},
            {  1,  0,  2,  4,  2,  3,  4,  3,  1,  0},
            {  1,  0,  0,  3,  1,  0,  3,  0,  0,  0},
            {  4,  2,  0,  0,  0,  4,  2,  3,  3,  4},
            {  0,  0,  4,  0,  4,  0,  4,  1,  0,  4},
            {  2,  2,  0,  0,  2,  0,  3,  2,  2,  3},
            {  3,  2,  1,  3,  4,  0,  0,  4,  3,  0},
            {  2,  4,  3,  4,  1,  4,  0,  3,  1,  4},
            {  0,  0,  2,  2,  3,  0,  3,  3,  3,  3},
            {  3,  0,  3,  2,  3,  3,  2,  4,  2,  4},
            {  0,  4,  1,  0,  4,  4,  3,  0,  2,  0},
            {  1,  4,  1,  2,  2,  1,  2,  1,  2,  2},
            {  4,  4,  4,  3,  1,  1,  1,  3,  2,  1},
            {  2,  1,  2,  1,  4,  1,  2,  0,  1,  1},
            {  0,  1,  1,  1,  3,  3,  3,  2,  1,  1},
            {  2,  0,  2,  0,  1,  1,  1,  2,  4,  0},
            {  3,  2,  4,  1,  1,  2,  4,  4,  2,  2},
            {  4,  2,  3,  1,  3,  1,  1,  4,  0,  0},
            {  3,  3,  1,  4,  0,  1,  4,  4,  0,  3},
            {  0,  4,  0,  2,  0,  0,  0,  4,  1,  0},
            {  3,  1,  0,  4,  4,  4,  1,  1,  0,  3},
            {  2,  0,  4,  1,  0,  3,  1,  0,  3,  2},
            {  0,  1,  2,  3,  1,  3,  2,  2,  0,  4},
            {  1,  2,  2,  0,  0,  3,  0,  1,  4,  1},
            {  4,  2,  1,  2,  1,  2,  1,  2,  1,  4},
            {  4,  1,  4,  4,  3,  4,  0,  1,  2,  0},
            {  2,  1,  1,  2,  2,  2,  4,  3,  0,  1},
            {  0,  3,  0,  3,  2,  1,  0,  2,  2,  2},
            {  1,  3,  3,  1,  1,  0,  2,  3,  2,  3},
            {  4,  0,  2,  4,  0,  2,  3,  4,  2,  1},
            {  0,  2,  3,  3,  2,  4,  4,  2,  3,  1},
            {  2,  3,  0,  0,  3,  1,  4,  1,  3,  1},
            {  3,  1,  3,  0,  0,  0,  1,  3,  4,  2},
            {  1,  4,  3,  0,  3,  2,  0,  0,  0,  2},
            {  2,  1,  4,  0,  2,  3,  0,  4,  1,  3},
            {  2,  3,  1,  3,  0,  2,  2,  1,  4,  0},
            {  3,  1,  3,  4,  0,  1,  3,  2,  3,  4},
            {  3,  3,  0,  2,  1,  3,  0,  0,  0,  1},
            {  1,  4,  4,  2,  4,  2,  1,  2,  3,  3},
            {  3,  0,  1,  3,  3,  4,  2,  1,  1,  3},
        };

        players_schedule = Schedule::createCustomScheduleFromPlayers(conf, player_tables);
    }

    std::unique_ptr<Schedule> final_schedule;
    bool optimize_seats = (argc > 6)
        ? strcmp(argv[6], "yes") == 0
        : false;
    if (optimize_seats)
    {
        printf("\n*** Optimize seats\n");
        size_t num_attempts = 50;
        size_t max_stages_seats = 2;
        size_t max_shuffle_per_stage = 500000;
        size_t max_switch_per_stage = 500000;

        double best_score = DBL_MAX;
        auto lambda = [&best_score](const Schedule& schedule, double score)
        {
            if (score < best_score) {
                best_score = score;
                printf("\n*** Current best SEAT schedule, score: %8.4f\n", score);
                outputFinal(schedule);
            }
            
            return true;
        };
        auto seats_schedule = solveSeats(
            *players_schedule, 
            num_attempts,
            max_stages_seats, 
            max_shuffle_per_stage,
            max_switch_per_stage,
            lambda);
        
        final_schedule = std::move(seats_schedule);
    }
    else
    {
        printf("\n*** Seat optimization skipped\n");
        final_schedule = std::move(players_schedule);
    }

    // print the best schedule
    printf("\n*** Final schedule\n");
    outputFinal(*final_schedule);

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
        sprintf(msg, "Number of games is too low: (%d, %d]", low_limit, high_limit);
        throw std::invalid_argument(msg);
    }

    if (games > high_limit) {
        static char msg[1024];
        sprintf(msg, "Number of games is too high: (%d, %d]", low_limit, high_limit);
        throw std::invalid_argument(msg);
    }

    // calc attempts
    if ((10 * games) % players != 0) {
        static char msg[1024];
        sprintf(msg, "Parameters mismatch. players_every_game(10) * games(%d) / players (%d) should be integer",
            games, players);
        throw std::invalid_argument(msg);
    }
    *out_attempts = 10 * games / players;
}
