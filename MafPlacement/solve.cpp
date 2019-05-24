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
        

        int k[11] = { 500, 100, 5, 1, 2, 50, 250, 500, 501, 502, 503 };
        add_penalty += metrics.aggregate(opponents, player, 
            [&k](int value)
            {               
                return k[value]; 
            });
    }

    return sd_penalty + add_penalty;
}

double calcSeatScore(const Schedule& schedule, Metrics& metrics)
{
    const auto& conf = schedule.config();

    double target = conf.numAttempts() / (double)Configuration::NumSeats;
    double sd_penalty = 0.0;

    double half_simmetry_penalty = 0.0;
    double triple_simmetry_penalty = 0.0;
    double first_penalty = 0.0;
    double last_penalty = 0.0;
    for (int player = 0; player < conf.numPlayers(); player++)
    {
        auto seats = metrics.calcPlayerSeatsHistogram(player);

        double sd = Metrics::calcSquareDeviation(seats, -1, target);
        sd_penalty += sd;
        
        assert(seats.size() == conf.NumSeats);
        double sum = seats[0] + seats[1] + 
            seats[2] + seats[3] + 
            seats[4] + seats[5] + 
            seats[6] + seats[7] + 
            seats[8] + seats[9];

        // half simetry
        {
            double lo = seats[0] + seats[1] + seats[2] + seats[3] + seats[4];
            double hi = seats[5] + seats[6] + seats[7] + seats[8] + seats[9];
            double k_lo = lo / sum;
            double k_hi = hi / sum;

            const double expected_lo = 0.5;
            const double expected_hi = 0.5;
            half_simmetry_penalty += fabs(k_lo - expected_lo) * fabs(k_lo - expected_lo);
            half_simmetry_penalty += fabs(k_hi - expected_hi) * fabs(k_hi - expected_hi);
        }

        // all seat numberes should be approximately even between
        // 1-3 : 30% 
        // 4-7 : 40%
        // 8-10: 30%
        {
            double a = seats[0] + seats[1] + seats[2];
            double b = seats[3] + seats[4] + seats[5] + seats[6];
            double c = seats[7] + seats[8] + seats[9];

            double k_a = a / sum;
            double k_b = b / sum;
            double k_c = c / sum;

            const double expected_a = 0.3;
            const double expected_b = 0.4;
            const double expected_c = 0.3;
            triple_simmetry_penalty += fabs(k_a - expected_a) * fabs(k_a - expected_a);
            triple_simmetry_penalty += fabs(k_b - expected_b) * fabs(k_b - expected_b);
            triple_simmetry_penalty += fabs(k_c - expected_c) * fabs(k_c - expected_c);
        }

        // Force seat1: 10% target
        // Force seat10: 10% target
        {
            const double expected_first = 0.1;
            const double expected_last = 0.1;
            double k_first = seats[0] / sum;
            double k_last = seats[9] / sum;
            first_penalty += fabs(k_first - expected_first) * fabs(k_first - expected_first);
            last_penalty += fabs(k_last - expected_last) * fabs(k_last - expected_last);
        }
    }

    return sd_penalty + half_simmetry_penalty + triple_simmetry_penalty 
        + 10 * first_penalty 
        + 10 * last_penalty;
}

std::unique_ptr<Schedule> solvePlayers(const Configuration& conf,
    size_t player_step, 
    size_t num_stages,
    size_t num_iterations,
    std::function<bool(const Schedule&, double)> schedule_fn)
{
    // calculate only ONE single initial schedule - just to reduce computations
    // assign this variable to ONE to get as many trivial initial schedules as possible
    if (player_step == 0)
        player_step = static_cast<player_t>(conf.numPlayers());

    printf("\n *** Player optimization\n");
    printf("\tPlayer_step: %zu\n", player_step);
    printf("\tNum stages: %zu\n", num_stages);
    printf("\tNum iterations on every stage: %zu\n", num_iterations);

    double best_score = FLT_MAX;
    double worst_score = FLT_MIN;
    std::unique_ptr<Schedule> best_schedule;

    for (player_t player_shift = 0; player_shift < conf.numPlayers(); player_shift += static_cast<player_t>(player_step)) {
        printf("\n* Player shift: %d\n", player_shift);
        for (size_t stage = 0; stage < num_stages; ++stage) {
            // create initial schedule
            std::unique_ptr<Schedule> schedule = Schedule::createInitialSchedule(conf, player_shift);

            Metrics metrics(*schedule);
            RandomOptimizer optimizer(*schedule, num_iterations,
                [&](const Schedule& s) { return calcPlayerScore(s, metrics); });
            double score = optimizer.optimize();
            
            size_t good_iterations = optimizer.goodIterations();
            size_t total_iterations = optimizer.totalIterations();
            printf("Stage: %3zu. Score: %10.2f. Iterations: %10zu / %10zu\n", 
                stage, score, 
                good_iterations, total_iterations);

            // callback - found a schedule
            if (!schedule_fn(*schedule, score))
                break;

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
    size_t num_shuffle_per_stage,
    size_t max_switch_per_stage,
    std::function<bool(const Schedule&, double)> schedule_fn)
{
    printf("\n *** Seat optimization\n");
    printf("Num stages: %zu\n", num_stages);
    printf("Num shuffles per stage: %zu\n", num_shuffle_per_stage);
    printf("Num switches per stage: %zu\n", max_switch_per_stage);

    const size_t max_attempts = 20;
    std::unique_ptr<Schedule> best_schedule;
    double best_score = FLT_MAX;

    for (size_t i = 0; i < max_attempts; i++)
    {
        printf("\nAttempt %3zu\n", i);
        Schedule schedule = initial_schedule;
        Metrics metrics(schedule);
        SeatOptimizer optimizer(schedule,
            num_stages,
            num_shuffle_per_stage,
            max_switch_per_stage,
            [&](const Schedule& s) { return calcSeatScore(s, metrics); });
        double score = optimizer.optimize();

        if (score < best_score)
        {
            best_score = score;
            best_schedule = std::make_unique<Schedule>(schedule);
            if (!schedule_fn(schedule, best_score))
                break;
        }
        
    }

    return best_schedule;
}

