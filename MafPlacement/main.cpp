#include <cstdio>
#include <cstdlib>

#include "core.h"

void usage()
{
    printf("Usage: <players> <tables> <rounds> <games>\n");
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        usage();
        return -1;
    }

    int players = (argc > 1)
        ? atoi(argv[1])
        : 0;

    int tables = (argc > 2)
        ? atoi(argv[2])
        : 0;

    int rounds = (argc > 3)
        ? atoi(argv[3])
        : 0;
    
    // Note: we probably don't need games, we can calculate it from players * tables * rounds
    int games = (argc > 4)
        ? atoi(argv[4])
        : 0;

    Configuration conf(players, tables, rounds, games);
    try {
        auto schedule = createSchedule(conf);
        printSchedule(schedule);
    }
    catch (std::exception& ex) {
        printf("Failed to create a schedule: %s", ex.what());
        return -1;
    }

    // TODO: optimize schedule to get better metrics

    
    return 0;
}

