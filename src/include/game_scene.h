#ifndef WAYTERRA_GAME_SCENE_H
#define WAYTERRA_GAME_SCENE_H

#include "game_objects.h"
#include <stddef.h>

#define MAX_GAME_OBJECTS 4096

typedef enum {
    GAME_OBJECT_SPRITE,
    GAME_OBJECT_ANIMATED_SPRITE,
    GAME_OBJECT_TILE,
} game_object_type_t;

typedef struct {
    game_object_type_t type;

    union {
        game_sprite_t sprite;
        animated_sprite_t animated_sprite;
        tile_t tile;
    } data;
} game_object_t;

typedef struct {
    game_object_t objects[MAX_GAME_OBJECTS];
    size_t count;
} game_scene_t;

#endif
