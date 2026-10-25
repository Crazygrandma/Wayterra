#ifndef WAYTERRA_GAME_INTERNAL_H
#define WAYTERRA_GAME_INTERNAL_H

#include "game.h"

game_object_t *game_scene_add_object(
    game_scene_t *scene,
    game_object_type_t type);

tile_t *game_add_tile(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite,
    int id);

animated_sprite_t *game_add_animated_sprite(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite,
    animation_t animation);

game_sprite_t *game_add_sprite(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite);

#endif
