#ifndef CONFIG_H
#define CONFIG_H

// TODO move config to parsed file

#include <wlr/types/wlr_keyboard.h>


// FIXME 
#define TILEMAP_HEIGHT 21.0
#define TILEMAP_WIDTH 21.0
#define NUM_TILES 30.0

// Edit to change the player walking speed
#define PLAYER_SPEED 5

// Edit to change the modifier key 
// WlR_MODIFIER_LOGO = Windows key
// WLR_MODIFIER_Alt = Alt key
#define MODIFIER_KEY WLR_MODIFIER_ALT
#define USER_KEYMAP "de"
#define KEY_REPEAT_RATE 50
#define KEY_REPEAT_DELAY 180
#endif
