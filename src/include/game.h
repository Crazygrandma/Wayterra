
#ifndef WAYTERRA_GAME_H
#define WAYTERRA_GAME_H

#include <stdbool.h>
#include <sprite.h>
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
    bool enableAnimation;
    bool inverted;
} animated_sprite_t;


typedef struct {
    float x;
    float y;

    wayterra_sprite_t sprite;
} wayterra_player_t;

typedef struct {
    wayterra_player_t player; 
    wayterra_sprite_t fireplace; 
    wayterra_sprite_t ground; 
    float backgroundX;

    double timer;
    double last_time;
    bool movement_mode;
} wayterra_game_t;

void game_init(wayterra_game_t *game);
void game_update(wayterra_game_t *game, float delta_time);

#endif
