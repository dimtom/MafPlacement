#include <cstdio>
#include <cstdlib>

#include <algorithm> 
#include <map>
#include <memory>

#include "core.h"
#include "metrics.h"

//
// forward declarations
//
void verifyParams(int players, int rounds, int tables, int games, int* out_attempts);
void outputInitial(const Schedule& schedule);
void outputFinal(const Schedule& schedule);

void optimizeRandom(Schedule& schedule);


//
// main
//
void usage()
{
    printf("Usage: <players> <rounds> <tables> <games>\n");
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
    outputInitial(*schedule);
    
    // optimize
    optimizeRandom(*schedule);

    // print final schedule
    outputFinal(*schedule);

    
    return 0;
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

void outputInitial(const Schedule& schedule)
{
    // print initial schedule
    printSchedule(schedule);

    if (!verifySchedule(schedule)) {
        throw std::exception("schedule is not valid");
    }
}

double calcScore(Schedule& schedule, Metrics& metrics)
{
    const auto& conf = schedule.config();

    double sdPenalty = 0.0;
    double addPenalty = 0.0;
    double target = 9.0 * conf.numAttempts() / (conf.numPlayers() - 1);
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
            
        double sd = Metrics::calcSquareDeviation(opponents, player, target);
        sdPenalty += sd;

        int k[11] = { 100, 50, 0, 0, 0, 0, 50, 100, 200, 400, 800};
        addPenalty += metrics.aggregate(opponents, player, [&k](int value) { return k[value]; });
    }

    return sdPenalty + addPenalty;
}

void optimizeRandom(Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // modify schedule
    const int NUM = 1000 * 1000;
    int div = 100;
    int num_success = 0;
    for (int i = 0; i < NUM; i++)
    {
        if ((i % div) == 0) {
            auto score = calcScore(schedule, metrics);
            printf("Iteration #%d: score=%6.2f good iterations: %d\n", i, score, num_success);
            div *= 2;
        }

        bool success = schedule.randomChange([&]() {
            return calcScore(schedule, metrics);
        });
        num_success += (int)success;
    }

    printf("Number of iterations: %d\n", NUM);
    printf("Successful iterations: %d\n", num_success);
}

void outputFinal(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // print schedule - temporarily disabled to reduce output
    // printSchedule(*schedule);

    if (!verifySchedule(schedule)) {
        throw std::exception("schedule is not valid");
    }

    printf("\nPlayer seats:\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto seats = metrics.calcPlayerSeatsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : seats)
            printf("%4d", v);
        printf("\n");
    }

    printf("\nPlayer opponents:\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : opponents)
            printf("%3d", v);
        printf("\n");
    }

    printf("\nPlayer statistics:\n");
    double target = 9.0 * conf.numAttempts() / (conf.numPlayers() - 1);
    printf("Each player should play %2.6f times with one another\n", target);
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        printf("Player %2d: ", player);

        int minv = Metrics::calcMin(opponents, player);
        int maxv = Metrics::calcMax(opponents, player);
        double avg = Metrics::calcAverage(opponents, player);

        double sd1 = Metrics::calcSquareDeviation(opponents, player);
        double sd2 = Metrics::calcSquareDeviation(opponents, player, target);

        printf("%3d %3d %2.6f %2.6f %2.6f\n", minv, maxv, avg, sd1, sd2);
    }
    
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

