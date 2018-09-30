#pragma once

#include <memory>

#include "configuration.h"
#include "game.h"
#include "schedule.h"

// creates and returns initial schedule for provided parameters (configuration)
std::unique_ptr<Schedule> 
createSchedule(const Configuration& conf, player_t shift_player_num);

bool verifySchedule(const Schedule& schedule);

void printConfiguration(const Configuration& conf);
void printSchedule(const Schedule& schedule);
void printGame(const Game& game);

