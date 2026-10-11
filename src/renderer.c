#include "renderer.h"
#include "renderer_utils.h"
#include "gl_utils.h"
#include <wlr/util/log.h>
#include <string.h>


static void make_quad(float *dst, float x0, float y0, float x1, float y1) {
    float quad[24] = {
        x0, y0, 0.0f, 0.0f,
        x1, y0, 1.0f, 0.0f,
        x1, y1, 1.0f, 1.0f,

        x0, y0, 0.0f, 0.0f,
        x1, y1, 1.0f, 1.0f,
        x0, y1, 0.0f, 1.0f
    };
    memcpy(dst, quad, sizeof(quad));
}

static void make_quad_pixels(
    float *dst,
    float x,
    float y,
    float width,
    float height,
    int screenWidth,
    int screenHeight)
{
    float x0 = 2.0f * x / screenWidth - 1.0f;
    float x1 = 2.0f * (x + width) / screenWidth - 1.0f;

    float y0 = 1.0f - 2.0f * y / screenHeight;
    float y1 = 1.0f - 2.0f * (y + height) / screenHeight;

    make_quad(dst, x0, y1, x1, y0);
}

static void init_shader(
    wayterra_shader_t *shader,
    const char *vertexPath,
    const char *fragmentPath)
{
    wlr_log(WLR_DEBUG, "Initializing shader");

    shader->program = create_shader_program_from_files(
        vertexPath,
        fragmentPath
    );

    if (!shader->program) {
        wlr_log(WLR_ERROR, "Failed to create shader program");
        return;
    }

    wlr_log(WLR_DEBUG, "Shader program: %u", shader->program);

    shader->posLoc = glGetAttribLocation(shader->program, "aPos");
    shader->uvLoc = glGetAttribLocation(shader->program, "aUV");
    shader->textureLoc = glGetUniformLocation(shader->program, "spriteTexture");
}

static void init_vertex_buffer(wayterra_renderer_t *r)
{
    wlr_log(WLR_DEBUG, "Creating vertex buffer");
    glGenBuffers(1, &r->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(r->vertices),
        r->vertices,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void init_textures( wayterra_renderer_t *r, wayterra_game_t *game) {
    r->backgroundTexture = load_texture("assets/background1.png");
    game->player.sprite.texture = load_texture("assets/player.png");
}

void initialize_renderer(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    wlr_log(WLR_DEBUG, "Initializing renderer");

    make_quad(r->vertices, -1.0f, -1.0f, 1.0f, 1.0f);

    make_quad_pixels(r->playerVerts, 1000.0f, 300.0f, 128.0f, 128.0f, width, height);

    init_shader(&r->spriteShader, "shader/vertex.glsl", "shader/fragment.glsl");

    if (!r->spriteShader.program) {
        return;
    }

    init_vertex_buffer(r);
    init_textures(r, game);

    r->shader_initialized = true;
}


static void setup_vertex_attributes(wayterra_renderer_t *r, wayterra_shader_t *shader)
{
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    GLsizei stride = 4 * sizeof(float);

    /* Position */
    glEnableVertexAttribArray(shader->posLoc);
    glVertexAttribPointer(
        shader->posLoc,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void *)0
    );

    /* UV */
    glEnableVertexAttribArray(shader->uvLoc);
    glVertexAttribPointer(
        shader->uvLoc,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void *)(2 * sizeof(float))
    );
}

static void draw_texture(
    wayterra_renderer_t *r,
    wayterra_shader_t *shader,
    GLuint texture,
    float *vertices)
{
    if (texture == 0) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(float) * 24,
        vertices
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(shader->textureLoc, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_background(wayterra_renderer_t *r)
{
    glDisable(GL_BLEND);

    draw_texture(
        r,
        &r->spriteShader,
        r->backgroundTexture,
        r->vertices
    );
}

static void draw_player(
    wayterra_renderer_t *r,
    wayterra_player_t *player)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    draw_texture(
        r,
        &r->spriteShader,
        player->sprite.texture,
        r->playerVerts
    );
}

static void use_shader(
    wayterra_renderer_t *r,
    wayterra_shader_t *shader)
{
    glUseProgram(shader->program);

    setup_vertex_attributes(r, shader);
}

static void stop_shader(wayterra_shader_t *shader)
{
    glDisableVertexAttribArray(shader->posLoc);
    glDisableVertexAttribArray(shader->uvLoc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void renderer_draw_frame(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    if (!r->shader_initialized) {
        initialize_renderer(r, game, width, height);
    }

    if (!r->shader_initialized) {
        wlr_log(WLR_ERROR,
            "Renderer initialization failed; skipping frame");
        return;
    }

    glViewport(0, 0, width, height);

    wayterra_shader_t *shader = &r->spriteShader;

    use_shader(r, shader);

    draw_background(r);
    draw_player(r, &game->player);

    stop_shader(shader);
}
