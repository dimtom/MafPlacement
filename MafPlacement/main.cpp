#include <cstdio>
#include <cstdlib>

#include <algorithm> 
#include <map>
#include <memory>

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

    // optimize player opponents
    // do not optimize seats yet
    /*printf("\n*** Optimize player opponents\n");
    size_t num_stages_players = 10;
    size_t max_iterations_players = 500 * 1000;
    auto players_schedule = solvePlayers(conf, num_stages_players, max_iterations_players);*/

    printf("\n*** Custom schedule\n");
    /*std::vector<std::vector<player_t>> custom_seats = {
        { 1,  2,  3,  4,  5, 11, 12, 13, 14, 15 },
        { 6,  7,  8,  9, 10, 16, 17, 18, 19, 20 },
        { 1,  3,  5,  7,  9, 11, 13, 15, 17, 19 },
        { 2,  4,  6,  8, 10, 12, 14, 16, 18, 20 },
        { 2,  5,  9,  3,  7,  13, 15, 12, 18, 19},
        { 1,  4,  8,  6, 10,  11, 14, 16, 17, 20}
    };*/

    std::vector<std::vector<player_t>> custom_seats = {
        { 1,  2,  3,  4,  5, 11, 12, 13, 14, 15 },
        { 6,  7,  8,  9, 10, 16, 17, 18, 19, 20 },
        { 1,  3,  5,  7,  9, 11, 13, 15, 17, 19 },
        { 2,  4,  6,  8, 10, 12, 14, 16, 18, 20 },
        { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 },
        {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10 }
    };
    auto players_schedule = Schedule::createCustomSchedule(conf, custom_seats);

    printf("\n*** After optimization\n");
    outputPlayerOptimization(*players_schedule);

    // don't switch players from-to games
    // do only in-game seat optimization
    //printf("\n*** Seats before\n");
    //outputSeatOptimization(*players_schedule);
    
    /*printf("\n*** Optimize seats\n");
    size_t max_stages_seats = 3;
    size_t max_iterations_seats = 1000 * 1000;
    auto seats_schedule = solveSeats(*players_schedule, max_stages_seats, max_iterations_seats);
    */

    //printf("\n*** Seats after\n");
    //outputSeatOptimization(*seats_schedule);

    // print the best schedule
    /*printf("\n*** Final schedule\n");
    outputFinal(*seats_schedule);
    */

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
