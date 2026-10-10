
#ifndef WAYTERRA_GAME_H
#define WAYTERRA_GAME_H

#include <stdbool.h>
#include <GLES2/gl2.h>

typedef struct {
    GLuint texture;
    float x;
    float y;
    float width;
    float height;
    bool animated;
} wayterra_player_t;

typedef struct {
} wayterra_world_t;

typedef struct {
    wayterra_player_t player;
    wayterra_world_t world;

    double timer;
    double last_time;
} wayterra_game_t;

void game_init(wayterra_game_t *game);
void game_update(wayterra_game_t *game, float delta_time);

#endif
