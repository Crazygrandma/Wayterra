
#include <time.h>
#include "game_world.h"

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
    game->timer += delta_time;
}
