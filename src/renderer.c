#include "renderer.h"
#include "file_utils.h"
#include "gl_utils.h"
#include <wlr/util/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void log_gl_error(const char *where)
{
    GLenum err;

    while ((err = glGetError()) != GL_NO_ERROR) {
        wlr_log(
            WLR_ERROR,
            "OpenGL error at %s: 0x%x",
            where,
            err
        );
    }
}

static void log_shader_info(GLuint shader, const char *name)
{
    GLint length = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    if (length > 1) {
        char *log = calloc(1, length);

        if (log) {
            glGetShaderInfoLog(
                shader,
                length,
                NULL,
                log
            );

            wlr_log(
                WLR_ERROR,
                "%s shader log:\n%s",
                name,
                log
            );

            free(log);
        }
    }
}

static void log_program_info(GLuint program)
{
    GLint length = 0;

    glGetProgramiv(
        program,
        GL_INFO_LOG_LENGTH,
        &length
    );

    if (length > 1) {
        char *log = calloc(1, length);

        if (log) {
            glGetProgramInfoLog(
                program,
                length,
                NULL,
                log
            );

            wlr_log(
                WLR_ERROR,
                "Program log:\n%s",
                log
            );

            free(log);
        }
    }
}


GLuint create_shader_program_from_files(
    const char *vertex_path,
    const char *fragment_path)
{
    wlr_log(
        WLR_DEBUG,
        "Loading vertex shader: %s",
        vertex_path
    );

    char *vs_src = load_file(vertex_path);

    if (!vs_src) {
        wlr_log(
            WLR_ERROR,
            "Failed to load vertex shader: %s",
            vertex_path
        );
        return 0;
    }

    wlr_log(
        WLR_DEBUG,
        "Loading fragment shader: %s",
        fragment_path
    );

    char *fs_src = load_file(fragment_path);

    if (!fs_src) {
        wlr_log(
            WLR_ERROR,
            "Failed to load fragment shader: %s",
            fragment_path
        );

        free(vs_src);
        return 0;
    }

    wlr_log(WLR_DEBUG, "Compiling vertex shader");

    GLuint vs = compile_shader(
        GL_VERTEX_SHADER,
        vs_src
    );

    log_gl_error("compile vertex shader");

    if (!vs) {
        wlr_log(
            WLR_ERROR,
            "Vertex shader compilation failed"
        );

        free(vs_src);
        free(fs_src);
        return 0;
    }

    log_shader_info(vs, "Vertex");

    wlr_log(WLR_DEBUG, "Compiling fragment shader");

    GLuint fs = compile_shader(
        GL_FRAGMENT_SHADER,
        fs_src
    );

    log_gl_error("compile fragment shader");

    if (!fs) {
        wlr_log(
            WLR_ERROR,
            "Fragment shader compilation failed"
        );

        glDeleteShader(vs);

        free(vs_src);
        free(fs_src);

        return 0;
    }

    log_shader_info(fs, "Fragment");

    free(vs_src);
    free(fs_src);

    wlr_log(WLR_DEBUG, "Creating shader program");

    GLuint program = glCreateProgram();

    log_gl_error("glCreateProgram");

    if (!program) {
        wlr_log(
            WLR_ERROR,
            "glCreateProgram returned 0"
        );

        glDeleteShader(vs);
        glDeleteShader(fs);

        return 0;
    }

    glAttachShader(program, vs);
    log_gl_error("glAttachShader vertex");

    glAttachShader(program, fs);
    log_gl_error("glAttachShader fragment");

    /*
     * IMPORTANT:
     *
     * If you want to explicitly assign locations, this must happen
     * BEFORE glLinkProgram().
     *
     * For now we're not doing that because we use
     * glGetAttribLocation() below.
     */

    wlr_log(WLR_DEBUG, "Linking shader program");

    glLinkProgram(program);

    log_gl_error("glLinkProgram");

    GLint success = GL_FALSE;

    glGetProgramiv(
        program,
        GL_LINK_STATUS,
        &success
    );

    log_gl_error("glGetProgramiv GL_LINK_STATUS");

    log_program_info(program);

    if (!success) {
        wlr_log(
            WLR_ERROR,
            "Shader program linking failed"
        );

        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);

        return 0;
    }

    wlr_log(
        WLR_DEBUG,
        "Shader program linked successfully: %u",
        program
    );

    /*
     * Shaders can be deleted after successful linking.
     */
    glDeleteShader(vs);
    log_gl_error("glDeleteShader vertex");

    glDeleteShader(fs);
    log_gl_error("glDeleteShader fragment");

    return program;
}


static void init_shader(wayterra_renderer_t *r)
{
    wlr_log(WLR_DEBUG, "Initializing renderer shader");

    static const float vertices[24] = {
        // position        // UV
        -1.0f, -1.0f,       0.0f, 0.0f,
         1.0f, -1.0f,       1.0f, 0.0f,
         1.0f,  1.0f,       1.0f, 1.0f,

        -1.0f, -1.0f,       0.0f, 0.0f,
         1.0f,  1.0f,       1.0f, 1.0f,
        -1.0f,  1.0f,       0.0f, 1.0f,
    };

    memcpy(
        r->vertices,
        vertices,
        sizeof(vertices)
    );

    wlr_log(
        WLR_DEBUG,
        "Creating shader program"
    );

    r->shader_program =
        create_shader_program_from_files(
            "shader/vertex.glsl",
            "shader/fragment.glsl"
        );

    if (!r->shader_program) {
        wlr_log(
            WLR_ERROR,
            "Failed to create shader program"
        );
        return;
    }

    wlr_log(
        WLR_DEBUG,
        "Shader program: %u",
        r->shader_program
    );

    r->pos_loc = glGetAttribLocation(
        r->shader_program,
        "aPos"
    );

    log_gl_error("glGetAttribLocation aPos");

    r->uv_loc = glGetAttribLocation(
        r->shader_program,
        "aUV"
    );

    log_gl_error("glGetAttribLocation aUV");

    wlr_log(
        WLR_DEBUG,
        "Shader attributes: aPos=%d aUV=%d",
        r->pos_loc,
        r->uv_loc
    );

    if (r->pos_loc < 0) {
        wlr_log(
            WLR_ERROR,
            "aPos was not found in shader"
        );
        return;
    }

    if (r->uv_loc < 0) {
        wlr_log(
            WLR_ERROR,
            "aUV was not found in shader"
        );
        return;
    }

    wlr_log(WLR_DEBUG, "Creating vertex buffer");

    glGenBuffers(1, &r->vbo);

    log_gl_error("glGenBuffers");

    wlr_log(
        WLR_DEBUG,
        "Created VBO: %u",
        r->vbo
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        r->vbo
    );

    log_gl_error("glBindBuffer GL_ARRAY_BUFFER");

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(r->vertices),
        r->vertices,
        GL_STATIC_DRAW
    );

    log_gl_error("glBufferData");

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    log_gl_error("glBindBuffer 0");

    wlr_log(
        WLR_DEBUG,
        "Shader initialization successful"
    );
}


void initialize_renderer(wayterra_renderer_t *r)
{
    wlr_log(WLR_DEBUG, "initialize_renderer()");

    init_shader(r);

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


    glViewport(
            0,
            0,
            width,
            height
            );


    glClearColor(
            1.0f,
            0.0f,
            0.0f,
            1.0f
            );


    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(r->shader_program);
    glBindBuffer(
            GL_ARRAY_BUFFER,
            r->vbo
            );
    GLsizei stride = 4 * sizeof(float);
    glEnableVertexAttribArray(r->pos_loc);


    glVertexAttribPointer(
            r->pos_loc,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void *)0
            );


    wlr_log(
            WLR_DEBUG,
            "Enabling UV attribute %d",
            r->uv_loc
           );

    glEnableVertexAttribArray(r->uv_loc);

    log_gl_error("glEnableVertexAttribArray uv");

    glVertexAttribPointer(
            r->uv_loc,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            (void *)(2 * sizeof(float))
            );



    glDrawArrays(
            GL_TRIANGLES,
            0,
            6
            );


    glDisableVertexAttribArray(r->pos_loc);

    glDisableVertexAttribArray(r->uv_loc);

    glBindBuffer(
            GL_ARRAY_BUFFER,
            0
            );


}
