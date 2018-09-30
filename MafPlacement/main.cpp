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

std::unique_ptr<Schedule> solvePlayers(const Configuration& conf);
void solveSeats(Schedule& schedule);

void outputInitial(const Schedule& schedule);
void outputPlayerOptimization(const Schedule& schedule);
void outputSeatOptimization(const Schedule& schedule);
void outputFinal(const Schedule& schedule);

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
    auto schedule = solvePlayers(conf);
    outputPlayerOptimization(*schedule);

    // don't switch players from-to games
    // do only in-game seat optimization
    outputSeatOptimization(*schedule);
    solveSeats(*schedule);
    outputSeatOptimization(*schedule);

    // print the best schedule
    outputFinal(*schedule);

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

        int k[11] = { 100, 50, 0, 0, 0, 0, 50, 100, 200, 400, 800 };
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

std::unique_ptr<Schedule> solvePlayers(const Configuration& conf)
{
    printf("\n*** Optimize player opponents\n");

    // calculate only ONE single initial schedule - just to reduce computations
    // assign this variable to ONE to get as many trivial initial schedules as possible
    player_t player_step = static_cast<player_t>(conf.numPlayers());

    std::vector<std::pair<std::unique_ptr<Schedule>, double>> schedules;
    for (player_t player_shift = 0; player_shift < conf.numPlayers(); player_shift += player_step) {

        // create initial schedule
        auto schedule = createSchedule(conf, player_shift);

        // optimize
        size_t max_iterations = 200 * 1000;
        Metrics metrics(*schedule);
        RandomOptimizer optimizer(*schedule, max_iterations, 
            [&](const Schedule& s) { return calcPlayerScore(s, metrics); });
        double score = optimizer.optimize();

        schedules.emplace_back(std::move(schedule), score);
    }

    // sort schedules
    std::sort(schedules.begin(), schedules.end(),
        [](const auto& a, const auto& b) {return a.second < b.second; });

    if (schedules.size() > 1) {
        printf("\nScores of optimized schedules:\n");
        for (const auto& item : schedules) {
            double score = item.second;
            printf("%8.4f\n", score);
        }
    }

    // return the best schedule
    return std::move(schedules.front().first);
}

void solveSeats(Schedule& schedule)
{
    printf("\n*** Optimize seats\n");

    size_t max_iterations = 10 * 1000 * 1000;
    Metrics metrics(schedule);
    SeatOptimizer optimizer(schedule, max_iterations, 
        [&](const Schedule& s) { return calcSeatScore(s, metrics); });
    optimizer.optimize();
}

// --------------------------------------------------------------------------
// print/output methods
// --------------------------------------------------------------------------

void outputInitial(const Schedule& schedule)
{
    // print initial schedule
    printSchedule(schedule);

    if (!verifySchedule(schedule)) {
        throw std::exception("schedule is not valid");
    }
}

void outputPlayerOptimization(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

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


void outputSeatOptimization(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    printf("\nPlayer seats:\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto seats = metrics.calcPlayerSeatsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : seats)
            printf("%4d", v);
        printf("\n");
    }
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

    printf("\nPlayer opponents:\n");
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        printf("Player %2d: ", player);
        for (const auto& v : opponents)
            printf("%3d", v);
        printf("\n");
    }
}






