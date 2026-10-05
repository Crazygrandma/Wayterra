#include "renderer_utils.h"

#include "file_utils.h"
#include "gl_utils.h"
#include <wlr/util/log.h>
#include <stdlib.h>


void log_gl_error(const char *where)
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
