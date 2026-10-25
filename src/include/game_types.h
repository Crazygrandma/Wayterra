#ifndef WAYTERRA_GAME_TYPES_H
#define WAYTERRA_GAME_TYPES_H

#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float z;

    float width;
    float height;
} transform_t;

typedef struct {
    const char *texturePath;
    unsigned int texture;
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

#endif
