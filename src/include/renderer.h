#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include <GLES2/gl2.h>

typedef struct wayterra_renderer {
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

    float vertices[24];
    float player_vertices[24];
    float logo_vertices[24];
} wayterra_renderer_t;

/* lifecycle */
void initialize_renderer(wayterra_renderer_t *r);
void renderer_shutdown(wayterra_renderer_t *r);

/* rendering */
void renderer_draw_frame(
    wayterra_renderer_t *r,
    const struct wayterra_game_state *state,
    int width,
    int height
);

/* optional helpers (render-only utilities) */
void renderer_update_uniforms(
    wayterra_renderer_t *r,
    int width,
    int height
);

void renderer_draw_player(
    wayterra_renderer_t *r,
    const struct wayterra_game_state *state
);

void renderer_draw_logo(
    wayterra_renderer_t *r
);

#endif
