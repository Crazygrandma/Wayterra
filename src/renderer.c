#include "renderer.h"
#include "gl_utils.h"
#include <wlr/util/log.h>

#include "renderers/sprite_renderer.h"
#include "renderers/animated_sprite_renderer.h"
#include "renderers/tilemap_renderer.h"

/* Virtual screen size used by the ortho projection and the world quad */
#define SCREEN_WIDTH  1920.0f
#define SCREEN_HEIGHT 1080.0f

static const float quadVertices[] = {
    // Position (x, y)                 // UV
    // Top-left
    0.0f,          0.0f,               0.0f, 0.0f,
    // Top-right
    1.0f,  0.0f,               1.0f, 0.0f,
    // Bottom-right
    1.0f,  1.0f,      1.0f, 1.0f,

    // Top-left
    0.0f,          0.0f,               0.0f, 0.0f,
    // Bottom-right
    1.0f,  1.0f,      1.0f, 1.0f,
    // Bottom-left
    0.0f,          1.0f,      0.0f, 1.0f
};

static void init_vertex_buffer(wayterra_renderer_t *r)
{
    wlr_log(WLR_DEBUG, "Creating vertex buffer");
    glGenBuffers(1, &r->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    glBufferData( GL_ARRAY_BUFFER, sizeof(quadVertices),
        quadVertices,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_textures(wayterra_renderer_t *r, wayterra_game_t *game)
{
    GLuint tileset = 0;

    for (size_t i = 0; i < game->scene.count; i++) {
        game_object_t *object = &game->scene.objects[i];
        sprite_t *sprite = NULL;

        switch (object->type) {
        case GAME_OBJECT_SPRITE:
            sprite = &object->data.sprite.sprite;
            break;

        case GAME_OBJECT_ANIMATED_SPRITE:
            sprite = &object->data.animated_sprite.sprite;
            break;

        case GAME_OBJECT_TILE:
            sprite = &object->data.tile.sprite;

            // All tiles share the same tileset texture.
            if (tileset == 0 && sprite->texturePath != NULL) {
                tileset = load_texture(sprite->texturePath);
            }

            sprite->texture = tileset;
            continue;
        }

        if (sprite == NULL || sprite->texturePath == NULL) {
            continue;
        }

        sprite->texture = load_texture(sprite->texturePath);
    }
}

void initialize_renderer(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    wlr_log(WLR_DEBUG, "Initializing renderer");


    init_sprite_shader(&r->spriteShader);
    init_animated_sprite_shader(&r->animatedSpriteShader);
    init_tilemap_shader(&r->tilemapShader);

    if (!r->spriteShader.base.program) {
        return;
    }
    if (!r->animatedSpriteShader.base.program) {
        return;
    }
    if (!r->tilemapShader.base.program) {
        return;
    }
    init_vertex_buffer(r);
    init_textures(r, game);

    r->initialized = true;
}


void renderer_draw_frame(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    if (!r->initialized) {
        initialize_renderer(r, game, width, height);
    }

    if (!r->initialized) {
        wlr_log(WLR_ERROR,
            "Renderer initialization failed; skipping frame");
        return;
    }

    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.298f, 0.267f, 0.650f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    tilemap_renderer_draw(r, game, width, height);
    sprite_renderer_draw(r, game, width, height);
    animated_sprite_renderer_draw(r, game, width, height);
}
