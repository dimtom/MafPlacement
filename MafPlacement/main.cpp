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

double calcScore(Configuration& conf, Schedule& schedule, Metrics& metrics)
{
    double score = 0.0;
    double target = 9.0 * conf.attempts() / conf.players();
    for (int player = 0; player < conf.players(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        double sd = Metrics::calcSquareDeviation(opponents, target);

        score += sd;
    }

    return score;
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

    

    // print initial schedule
    printSchedule(*schedule);

    if (!verifySchedule(*schedule)) {
        printf("### Schedule is not valid!\n");
        return -1;
    }

    Metrics metrics(*schedule);

    // modify schedule
    const int NUM = 1000 * 1000;
    int div = 100;
    int num_success = 0;
    for (int i = 0; i < NUM; i++)
    {
        if ((i % div) == 0) {
            auto score = calcScore(conf, *schedule, metrics);
            printf("Iteration #%d: score=%6.2f\n", i, score);
            div *= 2;
        }

        bool success = schedule->randomChange([&]() {
            return calcScore(conf, *schedule, metrics);
        });
        num_success += (int)success;
    }

    printf("Number of iterations: %d\n", NUM);
    printf("Successful iterations: %d\n", num_success);

    // print final schedule
    // printSchedule(*schedule);

    if (!verifySchedule(*schedule)) {
        printf("### Schedule is not valid!\n");
        return -1;
    }
    
    printf("\nPlayer seats:\n");
    for (int player = 0; player < conf.players(); player++)
    {
        auto seats = metrics.calcPlayerSeatsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : seats)
            printf("%4d", v);
        printf("\n");
    }

    printf("\nPlayer opponents:\n");
    for (int player = 0; player < conf.players(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : opponents)
            printf("%3d", v);
        printf("\n");
    }

    printf("\nPlayer statistics:\n");
    double target = 9.0 * conf.attempts() / conf.players();
    printf("Each player should play %2.6f times with one another\n", target);
    for (int player = 0; player < conf.players(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        printf("Player %2d: ", player);

        int minv = Metrics::calcMin(opponents);
        int maxv = Metrics::calcMax(opponents);
        double avg = Metrics::calcAverage(opponents);

        double sd1 = Metrics::calcSquareDeviation(opponents);
        double sd2 = Metrics::calcSquareDeviation(opponents, target);

        printf("%3d %3d %2.6f %2.6f %2.6f\n", minv, maxv, avg, sd1, sd2);
    }

    // TODO: optimize schedule to get better metrics

    
    return 0;
}

