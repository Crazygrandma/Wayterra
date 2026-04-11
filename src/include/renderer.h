// renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <GLES2/gl2.h>


typedef struct wayterra_renderer{

    GLuint shader_program;

    GLuint tileMapTexture;
    GLuint atlasTexture;
    GLuint playerSpriteTexture;
    GLuint logoSpriteTexture;

    GLint pos_loc;
    GLint uv_loc;

    GLint loc_renderMode;
    
    GLint loc_offset;
    GLint loc_tileMap;
    GLint loc_atlas;
    GLint loc_player;
    GLint loc_logo;
    GLint loc_windowWidth;
    GLint loc_windowHeight;

    GLint loc_tileMapSize;
    GLint loc_numTiles;

    bool shader_initialized;
    float vertices[24]; // 6 vertices * 4 floats (x, y, u, v)

    float player_vertices[24]; // same format: x, y, u, v

    float logo_vertices[24]; // same format: x, y, u, v
} wayterra_renderer_t;


void initialize_renderer(wayterra_renderer_t *r);
void renderer_draw_frame(wayterra_renderer_t *r, int width, int height);

#endif
