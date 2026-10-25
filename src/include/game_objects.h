#ifndef WAYTERRA_GAME_OBJECTS_H
#define WAYTERRA_GAME_OBJECTS_H

#include <stdbool.h>
#include "game_types.h"

typedef struct {
    transform_t transform;
    sprite_t sprite;
} game_sprite_t;

typedef struct {
    transform_t transform;
    sprite_t sprite;
    animation_t animation;
} animated_sprite_t;

typedef struct {
    transform_t transform;
    sprite_t sprite;
    int id;
} tile_t;

#endif
