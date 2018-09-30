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

//
// forward declarations
//
void verifyParams(int players, int rounds, int tables, int games, int* out_attempts);
void solve(int player, int rounds, int tables, int games, int attempts);
void outputInitial(const Schedule& schedule);
void outputFinal(const Schedule& schedule);
double optimizeRandom(Schedule& schedule);


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

    solve(players, rounds, tables, games, attempts);
    
    return 0;
}

void solve(int players, int rounds, int tables, int games, int attempts)
{
    // create config
    Configuration conf(players, rounds, tables, games, attempts);
    printConfiguration(conf);

    std::vector<std::pair<std::unique_ptr<Schedule>, double>> schedules;
    for (player_t player_shift = 0; player_shift < conf.numPlayers(); player_shift++) {

        // create initial schedule
        auto schedule = createSchedule(conf, player_shift);

        // print initial schedule
        // outputInitial(*schedule);

        // optimize
        double score = optimizeRandom(*schedule);

        schedules.emplace_back(std::move(schedule), score);
    }

    // sort schedules
    std::sort(schedules.begin(), schedules.end(),
        [](const auto& a, const auto& b) {return a.second < b.second; });

    printf("\nScores of optimized schedules:\n");
    for (const auto& item : schedules) {
        double score = item.second;
        printf("%8.4f\n", score);
    }

    // print the best schedule
    outputFinal(*schedules.front().first);
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

double calcScore(const Schedule& schedule, Metrics& metrics)
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

double optimizeRandom(Schedule& schedule)
{
    size_t max_iterations = 200 * 1000;
    RandomOptimizer optimizer(schedule, calcScore, max_iterations);
    double score = optimizer.optimize();
    return score;
}

void outputFinal(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // print schedule
    printSchedule(schedule);

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

