#include "config.h"
#include "game_objects.h"
#include "game_world.h"
#include <time.h>
#include <wlr/util/log.h>

static double get_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

static void resolve_tile_collision(animated_sprite_t *player, float tile_left,
                                   float tile_right, float tile_bottom,
                                   float tile_top, float *next_x, float *next_y,
                                   float velocity_x, float velocity_y) {
  float width = player->transform.width;
  float height = player->transform.height;

  // Horizontal collision
  if (*next_x < tile_right && *next_x + width > tile_left &&
      player->transform.y < tile_top &&
      player->transform.y + height > tile_bottom) {

    if (velocity_x > 0.0f) {
      *next_x = tile_left - width;
    } else if (velocity_x < 0.0f) {
      *next_x = tile_right;
    }
  }

  // Vertical collision
  if (*next_x < tile_right && *next_x + width > tile_left &&
      *next_y < tile_top && *next_y + height > tile_bottom) {

    if (velocity_y > 0.0f) {
      *next_y = tile_bottom - height;
    } else if (velocity_y < 0.0f) {
      *next_y = tile_top;
    }
  }
}

void game_init(wayterra_game_t *game) {
  *game = (wayterra_game_t){0};

  game->last_time = get_time();
  game->movement_mode = true;
  game->cameraZoom = 3.0f;

  game_world_init(game);
}

void game_update(wayterra_game_t *game, float delta_time) {
  if (game == NULL) {
    return;
  }

  game->timer += delta_time;

  if (game->player == NULL) {
    return;
  }

  float velocity_x = 0.0f;
  float velocity_y = 0.0f;

  if (game->move_right)
    velocity_x += PLAYER_SPEED;
  if (game->move_left)
    velocity_x -= PLAYER_SPEED;
  if (game->move_up)
    velocity_y += PLAYER_SPEED;
  if (game->move_down)
    velocity_y -= PLAYER_SPEED;

  // Normalize diagonal movement
  float length = sqrtf(velocity_x * velocity_x + velocity_y * velocity_y);

  if (length > 0.0f) {
    velocity_x = velocity_x / length * PLAYER_SPEED;
    velocity_y = velocity_y / length * PLAYER_SPEED;
  }

  float next_x = game->player->transform.x + velocity_x * delta_time;
  float next_y = game->player->transform.y + velocity_y * delta_time;

  // TODO add tilemap loading
  // Hardcoded tile at (0, 0)
  resolve_tile_collision(game->player, 0.0f, 16.0f, 0.0f, 16.0f, &next_x,
                         &next_y, velocity_x, velocity_y);

  game->player->transform.x = next_x;
  game->player->transform.y = next_y;

  // wlr_log(WLR_INFO, "Player transform: x=%.2f, y=%.2f", next_x, next_y);
  // wlr_log(WLR_INFO, "Player tile position transform: x=%.2f, y=%.2f",
  //         next_x / 16.f, next_y / 16.f);
}
