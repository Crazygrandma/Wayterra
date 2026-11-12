#ifndef CONFIG_H
#define CONFIG_H

// TODO move config to parsed file

#include <wlr/types/wlr_keyboard.h>

// FIXME
#define TILEMAP_HEIGHT 21.0
#define TILEMAP_WIDTH 21.0
#define NUM_TILES 30.0

// Edit to change the player walking speed
#define PLAYER_SPEED 300

// Edit to change how quickly horizontal velocity reaches PLAYER_SPEED
#define PLAYER_ACCEL 2400

// Edit to change gravity strength (pixels per second^2, y grows downward)
#define GRAVITY -1500

// Edit to change jump strength (initial upward velocity in pixels per second)
#define JUMP_SPEED 700

// Edit to change the maximum falling speed (pixels per second)
#define MAX_FALL_SPEED 1200

// Edit to change the modifier key
// WlR_MODIFIER_LOGO = Windows key
// WLR_MODIFIER_Alt = Alt key
#define MODIFIER_KEY WLR_MODIFIER_ALT
#define USER_KEYMAP "de"
#define KEY_REPEAT_RATE 50
#define KEY_REPEAT_DELAY 180
#endif
