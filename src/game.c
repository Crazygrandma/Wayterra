#include "game_internal.h"

#include <time.h>

#define TILE_WIDTH   32.0f
#define TILE_HEIGHT  32.0f

// Temporary player position and size.
#define PLAYER_X       0.0f
#define PLAYER_Y       0.0f
#define PLAYER_WIDTH   128.0f
#define PLAYER_HEIGHT  128.0f

// Seconds each player animation frame is shown.
#define PLAYER_SECONDS_PER_FRAME 0.1f

// Floor dimensions in tiles.
#define FLOOR_TILE_COLUMNS 50
#define FLOOR_TILE_ROWS    6

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

    for (int row = 0; row < FLOOR_TILE_ROWS; row++) {
        for (int column = 0; column < FLOOR_TILE_COLUMNS; column++) {
            game_add_tile(
                game,
                (transform_t){
                    .x = (float)column * TILE_WIDTH,
                    .y = (float)(FLOOR_TILE_ROWS - 1 - row) * TILE_HEIGHT,
                    .z = 0.0f,
                    .width = TILE_WIDTH,
                    .height = TILE_HEIGHT,
                },
                (sprite_t){
                    .texturePath = "assets/tilesetTexture.png",
                    .texture = 0,
                    .inverted = false,
                },
                row == 0 ? 1 : 0
            );
        }
    }

    // Single row of edge tiles above the top layer.
    for (int column = 0; column < FLOOR_TILE_COLUMNS; column++) {
        game_add_tile(
            game,
            (transform_t){
                .x = (float)column * TILE_WIDTH,
                .y = (float)FLOOR_TILE_ROWS * TILE_HEIGHT,
                .z = 0.0f,
                .width = TILE_WIDTH,
                .height = TILE_HEIGHT,
            },
            (sprite_t){
                .texturePath = "assets/tilesetTexture.png",
                .texture = 0,
                .inverted = false,
            },
            2
        );
    }

    // game_add_sprite(
    //     game,
    //     (transform_t){
    //         .x = SPRITE_X,
    //         .y = SPRITE_Y,
    //         .z = 0.0f,
    //         .width = SPRITE_WIDTH,
    //         .height = SPRITE_HEIGHT,
    //     },
    //     (sprite_t){
    //         .texturePath = "assets/tilesetTexture.png",
    //         .texture = 0,
    //         .inverted = false,
    //     }
    // );

    game->player = game_add_animated_sprite(
        game,
        (transform_t){
            .x = PLAYER_X,
            .y = PLAYER_Y,
            .z = 0.0f,
            .width = PLAYER_WIDTH,
            .height = PLAYER_HEIGHT,
        },
        (sprite_t){
            .texturePath = "assets/playerAnim.png",
            .texture = 0,
            .inverted = false,
        },
        (animation_t){
            .row = 0,
            .rowCount = 2,
            .frameCount = 11,
            .cellWidth = 64.0f,
            .cellHeight = 64.0f,
            .sheetWidth = 768.0f,
            .sheetHeight = 128.0f,
            .currentFrame = 0,
            .timeToNextFrame = PLAYER_SECONDS_PER_FRAME,
            .enabled = false,
        }
    );
}

void game_update(wayterra_game_t *game, float delta_time)
{
    game->timer += delta_time;
}
