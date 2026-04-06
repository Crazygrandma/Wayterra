#pragma once

#include <GLES2/gl2.h>
#include "stb/stb_header.h"

GLuint compile_shader(GLenum type, const char* src);
GLuint load_texture(const char *path);
