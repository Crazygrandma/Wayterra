
#ifndef WAYTERRA_RENDERER_H
#define WAYTERRA_RENDERER_H

#include <stdbool.h>
#include <GLES2/gl2.h>

#include "game.h"

// Sprite Shader
typedef struct {
    GLuint program;
    GLint posLoc;
    GLint uvLoc;
    GLint textureLoc;
} wayterra_shader_t;

typedef struct wayterra_renderer {
    GLuint vbo;
    GLuint fbo;

    wayterra_shader_t spriteShader;
   
    GLint backgroundTexture;
    bool shader_initialized;
    float vertices[24];
    float playerVerts[24];
} wayterra_renderer_t;

void initialize_renderer( wayterra_renderer_t *r, wayterra_game_t *game, int width, int height);

void renderer_draw_frame(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height
);

#endif
