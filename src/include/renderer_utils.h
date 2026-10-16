#ifndef WAYTERRA_RENDERER_UTILS_H
#define WAYTERRA_RENDERER_UTILS_H

#include <GLES2/gl2.h>
#include <cglm/cglm.h>
#include <renderer.h>


void setup_vertex_attributes(wayterra_renderer_t *r, sprite_shader_base_t *shader);

 /* OpenGL error / shader log helpers (utils/renderer_utils.c) */
void log_gl_error(const char *where);

/* Shader program compilation (utils/renderer_utils.c) */
GLuint create_shader_program_from_files(const char *vertex_path, const char *fragment_path);

//
void make_quad(
    float *dst,
    float x0, float y0,
    float x1, float y1
);

#endif
