#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <GLES2/gl2.h>

typedef struct wayterra_renderer {
    GLuint shader_program;

    GLuint vbo;
    
    GLuint fbo;

    GLint pos_loc;
    GLint uv_loc;

    bool shader_initialized;

    float vertices[24];
} wayterra_renderer_t;

/* lifecycle */
void initialize_renderer(wayterra_renderer_t *r);
void renderer_shutdown(wayterra_renderer_t *r);

/* rendering */
void renderer_draw_frame(
    wayterra_renderer_t *r,
    int width,
    int height
);

#endif
