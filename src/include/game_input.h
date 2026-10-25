#ifndef WAYTERRA_GAME_INPUT_H
#define WAYTERRA_GAME_INPUT_H

#include <stdbool.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-server-protocol.h>

#include "game.h"

bool game_handle_key(
    wayterra_game_t *game,
    xkb_keysym_t key,
    enum wl_keyboard_key_state state);

#endif
