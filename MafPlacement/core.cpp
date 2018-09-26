#include "core.h"
#include "schedule.h"

std::unique_ptr<Schedule> 
createSchedule(const Configuration& conf)
{
    auto schedule = std::make_unique<Schedule>(conf);

    // we need to create a schedule so that every player 
    // take part in equal number of games
    std::vector<int> games_played(conf.numPlayers(), conf.numAttempts());

    int game_num = 0;
    player_t player_num = 0;
    for (int r = 0; r < conf.numRounds(); r++) {
        for (int t = 0; t < conf.numTables(); t++) {
            if (game_num++ >= conf.numGames())
                break;

            // create and initialize a game
            std::vector<player_t> seats(conf.NumSeats, InvalidPlayerId);
            for (size_t i = 0; i < seats.size(); i++) {
                seats[i] = player_num;
                games_played[player_num]--;
                player_num = ++player_num % conf.numPlayers();
            }

            // add game to the schedule
            Game game(conf, seats);
            schedule->addGame(std::move(game));
        }
    }

    return schedule;
}

bool verifySchedule(const Schedule& schedule)
{
    // calc number of games played by every player
    std::vector<int> games_played(schedule.config().numPlayers());
    for (const auto& game : schedule.games())
    {
        for (auto id : game.seats())
        {
            games_played[id]++;
        }
    }

    // debug output
    printf("Sanity check: \n");
    for (size_t i = 0; i < games_played.size(); i++) {
        printf("%3d", games_played[i]);
    }
    printf("\n");

    int gp = games_played[0];
    for (size_t i = 1; i < games_played.size(); i++)
    {
        if (games_played[i] != gp)
            return false;
    }

    return true;



    // sanity check
    // every player
    /*for (size_t i = 0; i < games_played.size(); i++) {
    if (games_played[i] != 0) {
    char msg[80];
    sprintf_s(msg, "sanity check failed: games_played[%d]=%d", i, games_played[i]);
    throw std::exception(msg);
    }
    }*/
}

void printConfiguration(const Configuration& conf)
{
    printf("*** Configuration\n");
    printf("Players: %d\n", conf.numPlayers());
    printf("Rounds: %d\n", conf.numRounds());
    printf("Tables per round: %d\n", conf.numTables());
    printf("Total nunber of games: %d\n", conf.numGames());
    printf("Number of attempts (games played by each player during tournament): %d\n", conf.numAttempts());
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
    for (const auto& s : game.seats()) {
        printf("%3u", s);
    }
    printf("\n");
}