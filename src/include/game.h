
#ifndef WAYTERRA_GAME_H
#define WAYTERRA_GAME_H

#include <stdbool.h>
#include <GLES2/gl2.h>

typedef struct {
    GLuint texture;
    float width;
    float height;
} Sprite;

typedef struct {
    GLuint texture;
    float width;
    float height;
    float cellWidth;
    float cellHeight;
    float timeToNextFrame;
} AnimatedSprite;

typedef struct {
    AnimatedSprite sprite;
    float x;
    float y;
    float width;
    float height;
} wayterra_player_t;

typedef struct {
    wayterra_player_t player;
    Sprite logo;

    float backgroundX;

    double timer;
    double last_time;
} wayterra_game_t;

void game_init(wayterra_game_t *game);
void game_update(wayterra_game_t *game, float delta_time);

#endif
