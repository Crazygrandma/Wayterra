#pragma once

#include <GLES2/gl2.h>

/* OpenGL error / shader log helpers (utils/renderer_utils.c) */
void log_gl_error(const char *where);

/* Shader program compilation (utils/renderer_utils.c) */
GLuint create_shader_program_from_files(
    const char *vertex_path,
    const char *fragment_path
);
