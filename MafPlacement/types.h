#pragma once
#include <cstdint>

// id of player
typedef uint16_t player_t;

// id of player's position (seat) in the game
typedef uint8_t seat_t;

static const player_t InvalidPlayerId  = static_cast<uint16_t>(-1);
static const seat_t InvalidSeatId      = static_cast<uint8_t>(-1);
