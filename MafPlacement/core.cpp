#include "core.h"
#include "schedule.h"



Schedule createSchedule(const Configuration& conf)
{
    Schedule schedule(conf);

    // we need to create a schedule so that every player 
    // take part in equal number of games
    std::vector<int> games_played(conf.players(), conf.attempts());

    int game_num = 0;
    int player_num = 0;
    for (int r = 0; r < conf.rounds(); r++) {
        for (int t = 0; t < conf.tables(); t++) {

            // create and initialize a game
            Game game(game_num++);
            for (size_t i = 0; i < game.seats().size(); i++) {
                game.seats()[i] = player_num;
                games_played[player_num]--;
                player_num = ++player_num % conf.players();
            }

            // add game to the schedule
            schedule.addGame(std::move(game));
        }
    }

    // debug output
    printf("Sanity check: \n");
    for (size_t i = 0; i < games_played.size(); i++) {
        printf("%3d", games_played[i]);
    }
    printf("\n");

    // sanity check
    // every player
    /*for (size_t i = 0; i < games_played.size(); i++) {
        if (games_played[i] != 0) {
            char msg[80];
            sprintf_s(msg, "sanity check failed: games_played[%d]=%d", i, games_played[i]);
            throw std::exception(msg);
        }
    }*/

    return schedule;
}

void printConfiguration(const Configuration& conf)
{
    printf("*** Configuration\n");
    printf("Players: %d\n", conf.players());
    printf("Rounds: %d\n", conf.rounds());
    printf("Tables per round: %d\n", conf.tables());
    printf("Total nunber of games: %d\n", conf.games());
    printf("Number of attempts (games played by each player during tournament): %d\n", conf.attempts());
}

void printSchedule(const Schedule& schedule)
{
    printf("\n*** Schedule\n");

    int game_num = 0;
    for (const auto& game : schedule.games()) {
        printf("*** Game %3d >> ", ++game_num);
        printGame(game);
    }
}

void printGame(const Game& game)
{
    for (const auto& s : game.seats())
        printf("%3d", s);
    printf("\n");
}