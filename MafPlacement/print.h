#pragma once
#include "schedule.h"

void outputInitial(const Schedule& schedule);
void outputPlayerOptimization(const Schedule& schedule);
void outputSeatOptimization(const Schedule& schedule);
void outputFinal(const Schedule& schedule);

void printConfiguration(const Configuration& conf);
void printScheduleByGames(const Schedule& schedule);
void printSchedulebyRounds(const Schedule& schedule);
void printScheduleByPlayers(const Schedule& schedule);
void printScheduleByPlayersCStyle(const Schedule& schedule);
void printGame(const Game& game);
