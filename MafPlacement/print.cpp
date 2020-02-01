#include "print.h"
#include "metrics.h"

#include <cassert>

void outputPlayerMatrix(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    printf("\nPlayer opponents:\n");
    for (player_t player = 0; player < conf.numPlayers(); player++)
    {
        auto print_player = 1 + player;
        printf("Player %2d: ", print_player);
        
        auto opponents = metrics.calcPlayerOpponentsHistogram(player);
        assert(opponents.size() == conf.numPlayers());
        
        for (size_t i = 0; i < conf.numPlayers(); i++) {
            if (i != player)
                printf("%3d", opponents[i]);
            else
                printf("  *");
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

    if (!schedule.verify()) {
        throw std::runtime_error("schedule is not valid");
    }
}

void outputPlayerOptimization(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // histogram
    outputPairsHistogram(schedule);
    //outputPlayerStatistics(schedule);
    //outputPlayerMatrix(schedule);

    // print schedule
    //printSchedulebyRounds(schedule);
    printScheduleByPlayers(schedule);
    printScheduleByPlayersCStyle(schedule);
    if (!schedule.verify()) {
        throw std::runtime_error("schedule is not valid");
    }
}

void outputShortPlayerOptimization(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // histogram
    outputPairsHistogram(schedule);
    //outputPlayerStatistics(schedule);
    //outputPlayerMatrix(schedule);

    // print schedule
    //printSchedulebyRounds(schedule);
    //printScheduleByPlayers(schedule);
    printScheduleByPlayersCStyle(schedule);
    if (!schedule.verify()) {
        throw std::runtime_error("schedule is not valid");
    }
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

void outputFinal(const Schedule& schedule)
{
    Metrics metrics(schedule);
    const auto& conf = schedule.config();

    // statistics
    outputPairsHistogram(schedule);
    outputPlayerMatrix(schedule);
    outputPlayerStatistics(schedule);

    // print schedule
    printSchedulebyRounds(schedule);
    printScheduleByPlayers(schedule);
    printScheduleByPlayersCStyle(schedule);
    if (!schedule.verify()) {
        throw std::runtime_error("schedule is not valid");
    }

    outputSeatOptimization(schedule);
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
    printf("\n*** Schedule by games\n");

    int game_num = 0;
    for (const auto& game : schedule.games()) {
        game_num++;
        printf("Game %3d >> ", game_num);
        printGame(game);
    }
}

void printSchedulebyRounds(const Schedule& schedule)
{
    printf("\n*** Schedule by rounds\n");

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
    printf("\n*** Schedule for players\n");

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
                
                if (game->participates(player)) {
                    // found a table and a seat for this player this round
                    auto print_seat = 1 + game->players()[player];
                    char game_label = 'A' + game_num;
                    printf("%c:%-2d ", game_label, print_seat);
                    found = true;
                    break;
                }

                // 0-based game_num
                game_num++;
            }

            if (!found) {
                // no game for this player this round
                printf(" */*  ");
            }
        }
        printf("\n");
    }
}

void printScheduleByPlayersCStyle(const Schedule& schedule)
{
    printf("\n*** Schedule for players in C-style\n");

    auto& conf = schedule.config();
    for (player_t player = 0; player < conf.numPlayers(); player++) {
        auto print_player = 1 + player;
        printf("Player %2d: ", print_player);
        printf("{");

        bool comma = false;
        int round_num = 0;
        for (const auto& round : schedule.rounds()) {
            round_num++;

            if (comma)
                printf(",");
            comma = true;

            int game_num = 0;
            bool found = false;
            for (const Game* game : round.games()) {
                
                if (game->participates(player)) {
                    // found a table and a seat for this player this round
                    auto print_seat = 1 + game->players()[player];
                    printf(" %2d", game_num);
                    found = true;
                    break;
                }

                // 0-based game-num
                game_num++;
            }

            if (!found) {
                // no game for this player this round
                printf(" -1");
            }
        }
        printf("}\n");
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
