#include "game_world.h"
#include "game_internal.h"
#include <stdbool.h>

// Temporary player position and size.
#define PLAYER_X 30.0f
#define PLAYER_Y 0.0f
#define PLAYER_WIDTH 64.0f
#define PLAYER_HEIGHT 64.0f

// Seconds each player animation frame is shown.
#define PLAYER_SECONDS_PER_FRAME 0.1f

/* Virtual screen size used by the ortho projection and the world quad */
#define SCREEN_WIDTH 1920.0f
#define SCREEN_HEIGHT 1080.0f

void game_world_init(wayterra_game_t *game) {
  game->player =
      game_add_animated_sprite(game,
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
                               });

#define ADD_MARKER(x_pos, y_pos)                                               \
  game_add_sprite(game,                                                        \
                  (transform_t){                                               \
                      .x = x_pos,                                              \
                      .y = y_pos,                                              \
                      .z = 0.0f,                                               \
                      .width = MARKER_SIZE,                                    \
                      .height = MARKER_SIZE,                                   \
                  },                                                           \
                  (sprite_t){.texturePath = "assets/tilemapsand.png",          \
                             .texture = 0,                                     \
                             .inverted = false})

#define MARKER_SIZE 16.0f
#define MAP_WIDTH 40
#define MAP_HEIGHT 40

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1) {
        ADD_MARKER(x * MARKER_SIZE, y * MARKER_SIZE);
      }
    }
  }
}
