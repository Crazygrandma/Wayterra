
#ifndef WAYTERRA_GAME_H
#define WAYTERRA_GAME_H

#include <stdbool.h>
#include <sprite.h>
#include <GLES2/gl2.h>
#include <stddef.h>

#define MAX_GAME_OBJECTS 256

typedef struct {
    float x;
    float y;
    float z;

    float width;
    float height;
} transform_t;

typedef struct {
    const char *texturePath;
    GLuint texture;
    bool inverted;
} sprite_t;

typedef struct {
    int row;
    int rowCount;
    int frameCount;

    float cellWidth;
    float cellHeight;
    float sheetWidth;
    float sheetHeight;

    int currentFrame;
    float timeToNextFrame;

    bool enabled;
} animation_t;

typedef struct {
    transform_t transform;
    sprite_t sprite;
} game_sprite_t;

// Same params as wayterra_sprite_t, plus animation state
typedef struct {
    transform_t transform;
    sprite_t sprite;
    animation_t animation;
} animated_sprite_t;

#define MAX_GAME_OBJECTS 256

typedef struct {
    animated_sprite_t animated_objects[MAX_GAME_OBJECTS];
    size_t animated_count;

    game_sprite_t sprites[MAX_GAME_OBJECTS];
    size_t sprite_count;

    float playerX;
    float playerY;

    double timer;
    double last_time;
    bool movement_mode;
} wayterra_game_t;

void game_init(wayterra_game_t *game);
void game_update(wayterra_game_t *game, float delta_time);

#endif
