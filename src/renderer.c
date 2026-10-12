#include "renderer.h"
#include "renderer_utils.h"
#include "gl_utils.h"
#include <wlr/util/log.h>
#include <string.h>
#include <math.h>

/* Virtual screen size used by the ortho projection and the world quad */
#define SCREEN_WIDTH  1920.0f
#define SCREEN_HEIGHT 1080.0f

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

/*
 * Quad whose UVs are derived from world coordinates (uv = pos / tileSize),
 * so the texture repeats once per screen-sized tile. Requires the texture
 * to be created with GL_TEXTURE_WRAP_* = GL_REPEAT.
 */
static void make_tiled_quad(
    float *dst,
    float x0, float y0,
    float x1, float y1,
    float tileW, float tileH)
{
    float quad[24] = {
        x0, y0, x0 / tileW, y0 / tileH,
        x1, y0, x1 / tileW, y0 / tileH,
        x1, y1, x1 / tileW, y1 / tileH,

        x0, y0, x0 / tileW, y0 / tileH,
        x1, y1, x1 / tileW, y1 / tileH,
        x0, y1, x0 / tileW, y1 / tileH
    };
    memcpy(dst, quad, sizeof(quad));
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

    shader->timeLoc = glGetUniformLocation(shader->program, "uTime");
    shader->loc_animate = glGetUniformLocation(shader->program, "uAnimate");
    shader->projectionLoc = glGetUniformLocation(shader->program, "uProjection");
    shader->modelLoc = glGetUniformLocation(shader->program, "uModel");
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

    /*
     * load_texture() clamps; the background quad is oversized and scrolls,
     * so it needs to repeat. background1.png is 320x180 (power of two),
     * which GLES2 requires for GL_TEXTURE_REPEAT.
     */
    if (r->backgroundTexture) {
        glBindTexture(GL_TEXTURE_2D, r->backgroundTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    game->player.sprite.texture = load_texture("assets/player.png");
    // game->logo.texture = load_texture("assets/back.png");
}

void initialize_renderer(
    wayterra_renderer_t *r,
    wayterra_game_t *game,
    int width,
    int height)
{
    wlr_log(WLR_DEBUG, "Initializing renderer");

    // Hardcoded screen dimensions
    float screenWidth = SCREEN_WIDTH;
    float screenHeight = SCREEN_HEIGHT;

    // Center the player sprite
    float x = (screenWidth - game->player.width) / 2.0f;
    float y = (screenHeight - game->player.height) / 2.0f;
    // offset
    y += 100.f;

    /*
     * Oversized 3x3 tiled background. backgroundX is wrapped to
     * (-SCREEN_WIDTH, SCREEN_WIDTH) when it is applied, so this quad
     * always covers the visible area no matter how far the world scrolls.
     */
    make_tiled_quad(
        r->vertices,
        -screenWidth, -screenHeight,
        2.0f * screenWidth, 2.0f * screenHeight,
        screenWidth, screenHeight);

    make_quad(
        r->playerVerts,
        x, y,
        x + game->player.width,
        y + game->player.height
    );

    // Continue with your existing shader initialization...
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

    glClearColor(0.35f, 0.45f, 0.75f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    wayterra_shader_t *shader = &r->spriteShader;
    use_shader(r, shader);

    // Projection
    mat4 projection;
    glm_ortho(
            0.0f, SCREEN_WIDTH,
            0.0f, SCREEN_HEIGHT,
            -1.0f, 1.0f,
            projection
            );

    glUniformMatrix4fv(
            shader->projectionLoc,
            1,
            GL_FALSE,
            (const GLfloat *)projection
            );

    // Background: model matrix carries the world offset
    mat4 model;
    glm_mat4_identity(model);

    /*
     * Wrap the world offset to a single tile. The texture repeats every
     * SCREEN_WIDTH, so shifting by exactly one tile is invisible, and the
     * offset can never grow past the oversized quad's coverage.
     */
    float bg_x = fmodf(game->backgroundX, SCREEN_WIDTH);

    vec3 bg_position = {
        bg_x,
        0.0f,
        0.0f
    };

    glm_translate(model, bg_position);

    glUniformMatrix4fv(
            shader->modelLoc,
            1,
            GL_FALSE,
            (const GLfloat *)model
            );

    glDisable(GL_BLEND);
    draw_texture(r, shader, r->backgroundTexture, r->vertices);

    // Player: identity model matrix (stays put, world scrolls around it)
    glm_mat4_identity(model);

    glUniformMatrix4fv(
            shader->modelLoc,
            1,
            GL_FALSE,
            (const GLfloat *)model
            );

    glEnable(GL_BLEND);
    draw_texture(
            r,
            shader,
            game->player.sprite.texture,
            r->playerVerts
            );
    stop_shader(shader);
}
