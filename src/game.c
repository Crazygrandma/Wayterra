#include "game.h"
#include <time.h>

static animated_sprite_t *game_add_animated_sprite(
        wayterra_game_t *game,
        transform_t transform,
        sprite_t sprite,
        animation_t animation)
{
    if (game->animated_count >= MAX_GAME_OBJECTS) {
        return NULL;
    }

    animated_sprite_t *object = &game->animated_objects[game->animated_count++];

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

    game_sprite_t *object = &game->sprites[game->sprite_count++];

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

    float scale = 2.0f;

    // add player
    game_add_animated_sprite(
        game,
        (transform_t){
            .x = 100.0f,
            .y = 100.0f,
            .z = 0.0f,
            .width = 32.0f * scale,
            .height = 32.0f * scale,
        },
        (sprite_t){
        .texturePath = "assets/playeranim40x40.png",
        .texture = 0,
        .inverted = false,
        },
        (animation_t){
            .row = 0,
            .rowCount = 2,
            .frameCount = 1,
            .cellWidth = 40.0f,
            .cellHeight = 40.0f,
            .sheetWidth = 320.0f,
            .sheetHeight = 80.0f,
            .currentFrame = 0,
            .timeToNextFrame = 0.1f,
            .enabled = false,
        }
    );

    game_add_sprite(
            game,
            (transform_t){
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f,
            .width = 512.0f * scale,
            .height = 128.0f * scale,
            },
            (sprite_t){
            .texturePath = "assets/logov2.png",
            .inverted = false,
            .texture = 0,
            }
    );

    // add fireplace
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
            .frameCount = 19,
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
