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


static void init_shader(wayterra_renderer_t *r)
{

    wlr_log(WLR_DEBUG, "Initializing renderer shader");

    static const float vertices[24] = {
        // Position       // UV
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,

        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,
    };

    memcpy(r->vertices, vertices, sizeof(vertices));

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

    wlr_log(
        WLR_DEBUG,
        "Shader attributes: aPos=%d aUV=%d",
        r->pos_loc,
        r->uv_loc
    );

    if (r->pos_loc < 0) {
        wlr_log(WLR_ERROR, "aPos was not found in shader");
        return;
    }

    if (r->uv_loc < 0) {
        wlr_log(WLR_ERROR, "aUV was not found in shader");
        return;
    }

    r->time_loc = glGetUniformLocation(r->shader_program, "uTime");

    if (r->time_loc < 0) {
        wlr_log(WLR_ERROR, "uTime was not found in shader");
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

    log_gl_error("glBufferData");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    log_gl_error("glBindBuffer 0");

    wlr_log(WLR_DEBUG, "Shader initialization successful");
}


void initialize_renderer(wayterra_renderer_t *r)
{
    wlr_log(WLR_DEBUG, "initialize_renderer()");

    init_shader(r);

    // r->playerTexture = load_texture("assets/player.png");
    r->playerTexture = load_texture("assets/back.png");

    if (!r->shader_program ||
        r->pos_loc < 0 ||
        r->uv_loc < 0 ||
        !r->vbo) {

        wlr_log(
            WLR_ERROR,
            "Renderer initialization failed"
        );

        r->shader_initialized = false;
        return;
    }

    wlr_log(
        WLR_DEBUG,
        "Enabling alpha blending"
    );

    glEnable(GL_BLEND);
    log_gl_error("glEnable GL_BLEND");

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    log_gl_error("glBlendFunc");

    r->shader_initialized = true;

    wlr_log(
        WLR_DEBUG,
        "Renderer initialization complete"
    );
}


void renderer_draw_frame(
        wayterra_renderer_t *r,
        int width,
        int height)
{
    wlr_log(
            WLR_DEBUG,
            "renderer_draw_frame(%d, %d)",
            width,
            height
           );

    if (!r->shader_initialized) {
        wlr_log(
                WLR_DEBUG,
                "Renderer not initialized, initializing now"
               );

        initialize_renderer(r);
    }

    if (!r->shader_initialized) {
        wlr_log(
                WLR_ERROR,
                "Renderer initialization failed; skipping frame"
               );
        return;
    }

    glViewport(0, 0, width, height);

    // if (r->move_left) {
    //     glClearColor(0.0f, 0.0f, 0.25f, 1.0f);
    // } else if (r->move_right) {
    //     glClearColor(0.35f, 0.18f, 0.0f, 1.0f);
    // } else {
    //     glClearColor(0.25f, 0.0f, 0.0f, 1.0f);
    // }

    glClear(GL_COLOR_BUFFER_BIT);


    /*
     * Calculate frame time
     */
    double current_time = get_time();
    double delta_time = current_time - r->last_time;

    r->last_time = current_time;


    /*
     * Reverse time when moving left.
     * Move normally when moving right.
     */
    if (r->move_left) {
        delta_time = -delta_time;
    }

    r->timer += delta_time;


    /*
     * Use shader
     */
    glUseProgram(r->shader_program);


    /*
     * Draw player texture
     */
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(
            GL_TEXTURE_2D,
            r->playerTexture
            );

    glUniform1i(
            r->loc_player,
            0
            );


    /*
     * Vertex buffer
     */
    glBindBuffer(
            GL_ARRAY_BUFFER,
            r->vbo
            );

    GLsizei stride = 4 * sizeof(float);


    /*
     * Position
     */
    glEnableVertexAttribArray(r->pos_loc);

    glVertexAttribPointer(
            r->pos_loc,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void *)0
            );


    /*
     * Time
     */
    glUniform1f(
            r->time_loc,
            r->timer
            );


    /*
     * UV
     */
    wlr_log(
            WLR_DEBUG,
            "Enabling UV attribute %d",
            r->uv_loc
            );

    glEnableVertexAttribArray(r->uv_loc);

    log_gl_error(
            "glEnableVertexAttribArray uv"
            );

    glVertexAttribPointer(
            r->uv_loc,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void *)(2 * sizeof(float))
            );


    /*
     * Draw
     */
    glDrawArrays(
            GL_TRIANGLES,
            0,
            6
            );


    /*
     * Cleanup
     */
    glDisableVertexAttribArray(r->pos_loc);
    glDisableVertexAttribArray(r->uv_loc);

    glBindBuffer(
            GL_ARRAY_BUFFER,
            0
            );
}
