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

static void init_shader(wayterra_renderer_t *r)
{

    wlr_log(WLR_DEBUG, "Initializing renderer shader");

    wlr_log(WLR_DEBUG, "Creating shader program");

    r->shader_program = create_shader_program_from_files(
        "shader/vertex.glsl",
        "shader/fragment.glsl"
    );

    if (!r->shader_program) {
        wlr_log(WLR_ERROR, "Failed to create shader program");
        return;
    }

    wlr_log(WLR_DEBUG, "Shader program: %u", r->shader_program);

    r->pos_loc = glGetAttribLocation(r->shader_program, "aPos");
    r->uv_loc = glGetAttribLocation(r->shader_program, "aUV");
    r->time_loc = glGetUniformLocation(r->shader_program, "uTime");

    r->loc_texture = glGetUniformLocation( r->shader_program, "spriteTexture");
    r->loc_animate = glGetUniformLocation( r->shader_program, "uAnimate");


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

    game->player.texture = load_texture("assets/back.png");
}

void initialize_renderer( wayterra_renderer_t *r, wayterra_game_t *game)
{
    wlr_log(WLR_DEBUG, "initialize_renderer()");

    make_quad(r->vertices, -1.0f, -1.0f, 1.0f, 1.0f);

    init_shader(r);
    if (!r->shader_program) {
        return;
    }

    init_vertex_buffer(r);
    
    init_textures(r, game);


    r->shader_initialized = true;
}


static void setup_vertex_attributes(wayterra_renderer_t *r)
{
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);

    GLsizei stride = 4 * sizeof(float);

    /* Position */
    glEnableVertexAttribArray(r->pos_loc);
    glVertexAttribPointer(
        r->pos_loc,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void *)0
    );

    /* UV */
    glEnableVertexAttribArray(r->uv_loc);
    glVertexAttribPointer(
        r->uv_loc,
        2,
        GL_FLOAT,
        GL_FALSE,
        stride,
        (void *)(2 * sizeof(float))
    );
}

static void drawBackground(wayterra_renderer_t *r)
{
    glDisable(GL_BLEND);
    
    glUniform1i(r->loc_animate, 0);
    
    glActiveTexture(GL_TEXTURE0);

    glBindTexture( GL_TEXTURE_2D, r->backgroundTexture);

    glUniform1i( r->loc_texture, 0);

    glDrawArrays( GL_TRIANGLES, 0, 6);
}


static void drawPlayer(
    wayterra_renderer_t *r,
    wayterra_player_t *player)
{
    if (player->texture == 0) {
        return;
    }

    glEnable(GL_BLEND);

    glUniform1i(r->loc_animate, player->animated);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, player->texture);

    glUniform1i(r->loc_texture, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void renderer_draw_frame(
        wayterra_renderer_t *r,
        wayterra_game_t *game,
        int width,
        int height)
{
    if (!r->shader_initialized) {
        wlr_log(WLR_DEBUG,
            "Renderer not initialized, initializing now");

        initialize_renderer(r, game);
    }

    if (!r->shader_initialized) {
        wlr_log(WLR_ERROR,
            "Renderer initialization failed; skipping frame");

        return;
    }

    glViewport(0, 0, width, height);

    /* Activate the shader */
    glUseProgram(r->shader_program);

    /* Setup VBO and vertex attributes */
    setup_vertex_attributes(r);

    /* Set animation time before drawing */
    glUniform1f(r->time_loc, (float)game->timer);

    /* Draw background first */
    drawBackground(r);

    /* Draw player on top */
    drawPlayer(r, &game->player);

    /* Cleanup */
    glDisableVertexAttribArray(r->pos_loc);
    glDisableVertexAttribArray(r->uv_loc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
