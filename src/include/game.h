#ifndef WAYTERRA_GAME_H
#define WAYTERRA_GAME_H

#include <stdbool.h>
#include "game_scene.h"


typedef struct {
    game_scene_t scene;

    // The player points to an animated sprite inside the scene.
    animated_sprite_t *player;

    float playerX;
    float playerY;
    float cameraZoom;

    double timer;
    double last_time;

    bool movement_mode;

    bool move_right;
    bool move_left;
    bool move_up;
    bool move_down;
} wayterra_game_t;

void game_init(wayterra_game_t *game);
void game_update(wayterra_game_t *game, float delta_time);
#endif
