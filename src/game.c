#include "game.h"
#include <time.h>

static void game_update_player_animation(wayterra_game_t *game)
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
        // Don't restart the animation if already walking.
        if (player->animation.row != 1 ||
            !player->animation.enabled) {
            player->animation.row = 1;
            player->animation.frameCount = 8;
            player->animation.currentFrame = 0;
            player->animation.enabled = true;
        }
    } else {
        // Switch back to idle.
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
    bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;

    if (!game->movement_mode) {
        return false;
    }

    switch (key) {
        case XKB_KEY_a:
        case XKB_KEY_A:
            game->move_left = pressed;
            game_update_player_animation(game);
            return true;

        case XKB_KEY_d:
        case XKB_KEY_D:
            game->move_right = pressed;
            game_update_player_animation(game);
            return true;

        case XKB_KEY_w:
        case XKB_KEY_W:
            game->move_up = pressed;
            return true;

        case XKB_KEY_s:
        case XKB_KEY_S:
            game->move_down = pressed;
            return true;

        default:
            return false;
    }
}

static animated_sprite_t *game_add_animated_sprite(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite,
    animation_t animation)
{
    if (game->animated_count >= MAX_GAME_OBJECTS) {
        return NULL;
    }

    animated_sprite_t *object =
        &game->animated_objects[game->animated_count++];

    object->transform = transform;
    object->sprite = sprite;
    object->animation = animation;

    return object;
}

static game_sprite_t *game_add_sprite(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite)
{
    if (game->sprite_count >= MAX_GAME_OBJECTS) {
        return NULL;
    }

    game_sprite_t *object =
        &game->sprites[game->sprite_count++];

    object->transform = transform;
    object->sprite = sprite;

    return object;
}

static double get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void game_init(wayterra_game_t *game)
{
    *game = (wayterra_game_t){0};

    game->last_time = get_time();
    game->movement_mode = true;

    float scale = 8.0f;

    // Player: retain a pointer so input can change its animation.
    game->player = game_add_animated_sprite(
        game,
        (transform_t){
            .x = 100.0f,
            .y = 100.0f,
            .z = 0.0f,
            .width = 40.0f * scale,
            .height = 40.0f * scale,
        },
        (sprite_t){
            .texturePath = "assets/playerAnim.png",
            .texture = 0,
            .inverted = false,
        },
        (animation_t){
            .row = 0,
            .rowCount = 2,
            .frameCount = 10,
            .cellWidth = 64.0f,
            .cellHeight = 64.0f,
            .sheetWidth = 768.0f,
            .sheetHeight = 128.0f,
            .currentFrame = 0,
            .timeToNextFrame = 0.1f,
            .enabled = false,
        }
    );


    // Fireplace.
    game_add_animated_sprite(
        game,
        (transform_t){
            .x = 0.0f,
            .y = 100.0f,
            .z = 0.2f,
            .width = 16.0f * scale,
            .height = 32.0f * scale,
        },
        (sprite_t){
            .texturePath = "assets/campfireSpritesheet.png",
            .texture = 0,
            .inverted = false,
        },
        (animation_t){
            .row = 0,
            .rowCount = 1,
            .frameCount = 18,
            .cellWidth = 16.0f,
            .cellHeight = 32.0f,
            .sheetWidth = 304.0f,
            .sheetHeight = 32.0f,
            .currentFrame = 0,
            .timeToNextFrame = 0.1f,
            .enabled = true,
        }
    );
}

void game_update(wayterra_game_t *game, float delta_time)
{
    game->timer += delta_time;
}
