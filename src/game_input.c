#include "game_input.h"

static void update_player_animation(wayterra_game_t *game)
{
    animated_sprite_t *player = game->player;

    if (player == NULL) {
        return;
    }

    // Face the direction of movement.
    if (game->move_left) {
        player->sprite.inverted = true;
    } else if (game->move_right) {
        player->sprite.inverted = false;
    }

    bool walking = game->move_left || game->move_right;

    if (walking) {
        if (player->animation.row != 1 ||
            !player->animation.enabled) {
            player->animation.row = 1;
            player->animation.frameCount = 8;
            player->animation.currentFrame = 0;
            player->animation.enabled = true;
        }
    } else {
        if (player->animation.row != 0 ||
            player->animation.enabled) {
            player->animation.row = 0;
            player->animation.frameCount = 11;
            player->animation.currentFrame = 0;
            player->animation.enabled = false;
        }
    }
}

bool game_handle_key(
    wayterra_game_t *game,
    xkb_keysym_t key,
    enum wl_keyboard_key_state state)
{
    if (game == NULL) {
        return false;
    }

    bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;

    switch (key) {
    case XKB_KEY_a:
    case XKB_KEY_A:
        game->move_left = pressed;
        break;

    case XKB_KEY_d:
    case XKB_KEY_D:
        game->move_right = pressed;
        break;

    case XKB_KEY_w:
    case XKB_KEY_W:
        game->move_up = pressed;
        break;

    case XKB_KEY_s:
    case XKB_KEY_S:
        game->move_down = pressed;
        break;

    default:
        return false;
    }

    // float dx = 0.0f;
    // float dy = 0.0f;
    //
    // dx -= PLAYER_SPEED;
    // dx += PLAYER_SPEED;
    //
    // dy += PLAYER_SPEED;
    // dy -= PLAYER_SPEED;
    //
    // if (player != NULL) {
    //     player->transform.x += dx;
    //     player->transform.y += dy;
    // }
    update_player_animation(game);
    return true;
}
