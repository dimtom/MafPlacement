#include "solve.h"

#include "metrics.h"
#include "random_optimizer.h"
#include "seat_optimizer.h"

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

        // int k[11] = { 100, 50, 0, 0, 0, 0, 20, 100, 200, 400, 800 };
        // int k[11] = { 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        // int k[11] = { 500, 100, 10, 0, 10, 50, 150, 200, 400, 800, 1000 };

        int k[4] = { 300, 0, 0, 0 };
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
            std::unique_ptr<Schedule> schedule;
            if (player_shift == 0) {
                printf("\n*** Custom schedule\n");
                std::vector<std::vector<player_t>> custom_seats = {
                    { 1,  2,  3,  4,  5, 11, 12, 13, 14, 15 },
                    { 6,  7,  8,  9, 10, 16, 17, 18, 19, 20 },
                    { 1,  3,  5,  7,  9, 11, 13, 15, 17, 19 },
                    { 2,  4,  6,  8, 10, 12, 14, 16, 18, 20 },
                    //{ 2,  5,  9,  3,  7,  13, 15, 12, 18, 19 },
                    //{ 1,  4,  8,  6, 10,  11, 14, 16, 17, 20 }
                    { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 },
                    { 1,  2,  3,  4,  5,  6, 7, 8, 9, 10 }
                };
                schedule = Schedule::createCustomSchedule(conf, custom_seats);
            }
            else {
                schedule = Schedule::createInitialSchedule(conf, player_shift);
            }

            // print initial schedule
            // outputInitial(*schedule);

            Metrics metrics(*schedule);
            RandomOptimizer optimizer(*schedule, num_iterations,
                [&](const Schedule& s) { return calcPlayerScore(s, metrics); });
            double score = optimizer.optimize();
            
            size_t good_iterations = optimizer.goodIterations();
            size_t total_iterations = optimizer.totalIterations();
            printf("Stage: %3zu. Score: %10.2f. Iterations: %10zu / %10zu\n", 
                stage, score, 
                good_iterations, total_iterations);

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

