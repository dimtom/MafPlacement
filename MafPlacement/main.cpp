#include <cstdio>
#include <cstdlib>

#include <memory>

#include "core.h"
#include "metrics.h"

void usage()
{
    printf("Usage: <players> <rounds> <tables> <games>\n");
}

void verifyParams(int players, int rounds, int tables, int games, int attempts)
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

    if (players * attempts != 10 * games) {
        static char msg[1024];
        sprintf_s(msg, "Parameters mismatch. players(%d) * attempts(%d) != players_every_game(10) * games(%d)", 
            players, attempts, games);
        throw std::invalid_argument(msg);
    }
    
}

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

    int attempts = 10 * games / players;

    // verify params
    try {
        verifyParams(players, rounds, tables, games, attempts);
    }
    catch (std::exception& ex) {
        printf("Failed to verify parameters:\n%s", ex.what());
        return -1;
    }

    // create config
    Configuration conf(players, rounds, tables, games, attempts);
    printConfiguration(conf);

    // create initial schedule
    std::unique_ptr<Schedule> schedule;
    try {
        schedule = createSchedule(conf);
    }
    catch (std::exception& ex) {
        printf("Failed to create initial schedule: %s", ex.what());
        return -1;
    }

    // print schedule
    printSchedule(*schedule);

    Metrics metrics(*schedule);
    printf("\nPlayer seats:\n");
    for (int player = 0; player < conf.players(); player++)
    {
        auto seats = metrics.CalcPlayerSeatsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : seats)
            printf("%4d", v);
        printf("\n");
    }

    printf("\nPlayer opponents:\n");
    for (int player = 0; player < conf.players(); player++)
    {
        auto opponents = metrics.CalcPlayerOpponentsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : opponents)
            printf("%3d", v);
        printf("\n");
    }

    // TODO: optimize schedule to get better metrics

    
    return 0;
}

