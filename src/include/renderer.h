
#ifndef WAYTERRA_RENDERER_H
#define WAYTERRA_RENDERER_H

#include <stdbool.h>
#include <GLES2/gl2.h>

#include "game.h"

typedef struct wayterra_renderer {
    GLuint shader_program;
    GLuint vbo;
    GLuint fbo;

    GLint pos_loc;
    GLint uv_loc;
    GLint time_loc;
    GLint loc_texture;
    GLint loc_animate;

    bool shader_initialized;
    float vertices[24];

    GLuint backgroundTexture;
} wayterra_renderer_t;

void initialize_renderer(wayterra_renderer_t *r, wayterra_game_t *game);

void renderer_draw_frame(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height
);

#endif
