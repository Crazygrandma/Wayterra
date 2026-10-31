#include <time.h>
#include "game_world.h"
#include "config.h"

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

    game_world_init(game);
}

void game_update(wayterra_game_t *game, float delta_time)
{
    if (game == NULL) {
        return;
    }

    game->timer += delta_time;

    if (game->player == NULL) {
        return;
    }

    float dx = 0.0f;
    float dy = 0.0f;

    if (game->move_left)
        dx -= 1.0f;

    if (game->move_right)
        dx += 1.0f;

    if (game->move_up)
        dy += 1.0f;

    if (game->move_down)
        dy -= 1.0f;

    game->player->transform.x += dx * PLAYER_SPEED * delta_time;
    game->player->transform.y += dy * PLAYER_SPEED * delta_time;
}
