#include "renderer.h"
#include "renderer_utils.h"
#include "gl_utils.h"
#include <wlr/util/log.h>
#include <string.h>
#include <time.h>

static double get_time(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

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
    log_gl_error("glGetAttribLocation aPos");

    r->uv_loc = glGetAttribLocation(r->shader_program, "aUV");
    log_gl_error("glGetAttribLocation aUV");


    if (r->pos_loc < 0) {
        wlr_log(WLR_ERROR, "aPos was not found in shader");
        return;
    }

    if (r->uv_loc < 0) {
        wlr_log(WLR_ERROR, "aUV was not found in shader");
        return;
    }


    r->loc_player = glGetUniformLocation(
        r->shader_program,
        "playerTexture"
    );

    wlr_log(WLR_DEBUG, "Creating vertex buffer");

    glGenBuffers(1, &r->vbo);
    log_gl_error("glGenBuffers");

    wlr_log(WLR_DEBUG, "Created VBO: %u", r->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    log_gl_error("glBindBuffer GL_ARRAY_BUFFER");

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(r->vertices),
        r->vertices,
        GL_STATIC_DRAW
    );


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    wlr_log(WLR_DEBUG, "Shader initialization successful");
}

static void init_textures(wayterra_renderer_t *r) {
    r->backgroundTexture = load_texture("assets/background1.png");
    r->playerTexture = load_texture("assets/back.png");
}

void initialize_renderer(wayterra_renderer_t *r)
{
    wlr_log(WLR_DEBUG, "initialize_renderer()");

    /* Must run before init_shader(), which uploads r->vertices to the VBO */
    make_quad(r->vertices, -1.0f, -1.0f, 1.0f, 1.0f);   // fullscreen

    init_shader(r);
    init_textures(r);

    // Enable alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    r->shader_initialized = true;

    wlr_log(
        WLR_DEBUG,
        "Renderer initialization complete"
    );
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
    glActiveTexture(GL_TEXTURE0);

    glBindTexture( GL_TEXTURE_2D, r->backgroundTexture);

    glUniform1i( r->loc_player, 0);

    glDrawArrays( GL_TRIANGLES, 0, 6);
}

static void drawPlayer(wayterra_renderer_t *r)
{
    glActiveTexture(GL_TEXTURE0);

    glBindTexture( GL_TEXTURE_2D, r->playerTexture);

    glUniform1i( r->loc_player, 0);

    glDrawArrays( GL_TRIANGLES, 0, 6);
}

void renderer_draw_frame(
        wayterra_renderer_t *r,
        int width,
        int height)
{
    if (!r->shader_initialized) {
        wlr_log( WLR_DEBUG, "Renderer not initialized, initializing now");

        initialize_renderer(r);
    }

    if (!r->shader_initialized) {
        wlr_log( WLR_ERROR, "Renderer initialization failed; skipping frame");

        return;
    }

    glViewport( 0, 0, width, height);

    /* Clear framebuffer */
    // glClearColor( 1.0f, 0.0f, 1.0f, 1.0f);
    //
    // glClear(GL_COLOR_BUFFER_BIT);

    /* Use shader */
    glUseProgram(r->shader_program);

    /* Setup VBO and vertex attributes */
    setup_vertex_attributes(r);

    /* Draw background first */
    drawBackground(r);

    /* Draw player on top */
    drawPlayer(r);

    /* Cleanup */
    glDisableVertexAttribArray(r->pos_loc);
    glDisableVertexAttribArray(r->uv_loc);

    glBindBuffer( GL_ARRAY_BUFFER, 0);
}
