#pragma once

#include "configuration.h"
#include "game.h"
#include "schedule.h"

Schedule createSchedule(const Configuration& conf);

void printConfiguration(const Configuration& conf);
void printSchedule(const Schedule& schedule);
void printGame(const Game& game);

