#include "core.h"
#include "schedule.h"

std::unique_ptr<Schedule> 
createSchedule(const Configuration& conf, player_t shift_player_num = 0)
{
    // we need to create a schedule so that every player 
    // take part in equal number of games
    std::vector<int> num_games_played(conf.numPlayers(), static_cast<int>(conf.numAttempts()));

    int game_num = 0;
    player_t player_num = shift_player_num % conf.numPlayers();
    std::vector<Game> games;
    for (int r = 0; r < conf.numRounds(); r++) {
        for (int t = 0; t < conf.numTables(); t++) {
            if (game_num++ >= conf.numGames())
                break;

            // create and initialize a game
            std::vector<player_t> seats(conf.NumSeats, InvalidPlayerId);
            for (size_t i = 0; i < seats.size(); i++) {
                seats[i] = player_num;
                num_games_played[player_num]--;
                player_num = ++player_num % conf.numPlayers();
            }

            games.emplace_back(conf, seats);
        }
    }

    auto schedule = std::make_unique<Schedule>(conf, games);
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
    
    bool ok = true;
    int gp = games_played[0];
    for (size_t i = 1; i < games_played.size(); i++)
    {
        if (games_played[i] != gp) {
            ok = false;
            break;
        }
    }

    // debug output only if we have problems
    if (!ok) {
        
        printf("Sanity check: \n");
        for (size_t i = 0; i < games_played.size(); i++) {
            printf("%3d", games_played[i]);
        }
        printf("\n");
    }

    return ok;
}