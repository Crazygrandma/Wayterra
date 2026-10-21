
#ifndef WAYTERRA_GAME_H
#define WAYTERRA_GAME_H

#include <stdbool.h>
#include <sprite.h>
#include <GLES2/gl2.h>
#include <stddef.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-server-protocol.h>
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

// A single tile of the tilemap. Every tile samples the same tileset
// texture, `id` selects which tile of the tileset is drawn.
typedef struct {
    transform_t transform;
    sprite_t sprite;
    int id;
} tile_t;

#define MAX_GAME_OBJECTS 256

typedef struct {
    animated_sprite_t animated_objects[MAX_GAME_OBJECTS];
    size_t animated_count;

    animated_sprite_t *player;
    game_sprite_t sprites[MAX_GAME_OBJECTS];
    size_t sprite_count;

    tile_t tiles[MAX_GAME_OBJECTS];
    size_t tile_count;

    float playerX;
    float playerY;

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

bool game_handle_key( wayterra_game_t *game, xkb_keysym_t key, enum wl_keyboard_key_state state);
#endif
