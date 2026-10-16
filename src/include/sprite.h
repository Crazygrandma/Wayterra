#ifndef WAYTERRA_SPRITE_H
#define WAYTERRA_SPRITE_H

#include <GLES2/gl2.h>

typedef struct {
    GLuint texture;

    float x;
    float y;
    float z;
    float width;
    float height;
} wayterra_sprite_t;

#endif
