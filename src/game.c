#include "game.h"
#include <time.h>

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

    // TODO apply with camera scale
    float scaleFactor = 6.f;

    // player woorld coords
    game->player.x = 10.0f;
    game->player.y = 0.0f; 

    game->fireplace.x = 100.f;
    game->fireplace.y = 100.f;
    game->fireplace.width = 16.f * scaleFactor;
    game->fireplace.height = 32.f * scaleFactor;

    game->ground.x = -300.f;
    game->ground.y = -260.f;
    game->ground.z = 0.5f;
    game->ground.width = 320.f * scaleFactor;
    game->ground.height = 64.f  * scaleFactor;

    game->player.sprite.x = 100.f;
    game->player.sprite.y = 100.f;
    game->player.sprite.z = 0.0f;
    // Set dimensions
    game->player.sprite.width = 32.f * scaleFactor;
    game->player.sprite.height = 32.f * scaleFactor;
}

void game_update(wayterra_game_t *game, float delta_time)
{
    game->timer += delta_time;

}
