
#ifndef WAYTERRA_RENDERER_H
#define WAYTERRA_RENDERER_H

#include <stdbool.h>
#include <GLES2/gl2.h>
#include "game.h"

typedef struct {
    GLuint program;
    GLint posLoc;
    GLint uvLoc;

    GLint projectionLoc;
    GLint modelLoc;
    GLint viewLoc;
} sprite_shader_base_t;

// Sprite Shader
typedef struct {
    sprite_shader_base_t base;
    GLint textureLoc;
} sprite_shader_t;

typedef struct {
    sprite_shader_base_t base;
    GLint loc_animationTexture;
    GLint loc_animate;
    GLint loc_time;
    GLint loc_inverted;
    GLint loc_row;
    GLint loc_rowCount;
    GLint loc_frameCount;
    GLint loc_timePerFrame;
    GLint loc_cellWidthUV;
} animated_sprite_shader_t;

typedef struct{
    sprite_shader_base_t base;
    GLint textureLoc;
    GLint loc_tile_id;
} tilemap_shader_t;

typedef struct wayterra_renderer {
    GLuint vbo;
    GLuint fbo;

    sprite_shader_t spriteShader;
    animated_sprite_shader_t animatedSpriteShader;
    tilemap_shader_t tilemapShader;
   
    GLint backgroundTexture;
    bool initialized;
} wayterra_renderer_t;

void initialize_renderer( wayterra_renderer_t *r, wayterra_game_t *game, int width, int height);

void renderer_draw_frame( wayterra_renderer_t *r, wayterra_game_t *game, int width, int height);

#endif
