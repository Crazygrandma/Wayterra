#include "game_internal.h"
game_object_t *game_scene_add_object(
    game_scene_t *scene,
    game_object_type_t type)
{
    if (scene == NULL || scene->count >= MAX_GAME_OBJECTS) {
        return NULL;
    }

    game_object_t *object = &scene->objects[scene->count++];

    *object = (game_object_t){
        .type = type,
    };

    return object;
}

tile_t *game_add_tile(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite,
    int id)
{
    game_object_t *object = game_scene_add_object(
        &game->scene,
        GAME_OBJECT_TILE);

    if (object == NULL) {
        return NULL;
    }

    object->data.tile = (tile_t){
        .transform = transform,
        .sprite = sprite,
        .id = id,
    };

    return &object->data.tile;
}

animated_sprite_t *game_add_animated_sprite(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite,
    animation_t animation)
{
    game_object_t *object = game_scene_add_object(
        &game->scene,
        GAME_OBJECT_ANIMATED_SPRITE);

    if (object == NULL) {
        return NULL;
    }

    object->data.animated_sprite = (animated_sprite_t){
        .transform = transform,
        .sprite = sprite,
        .animation = animation,
    };

    return &object->data.animated_sprite;
}

game_sprite_t *game_add_sprite(
    wayterra_game_t *game,
    transform_t transform,
    sprite_t sprite)
{
    game_object_t *object = game_scene_add_object(
        &game->scene,
        GAME_OBJECT_SPRITE);

    if (object == NULL) {
        return NULL;
    }

    object->data.sprite = (game_sprite_t){
        .transform = transform,
        .sprite = sprite,
    };

    return &object->data.sprite;
}
