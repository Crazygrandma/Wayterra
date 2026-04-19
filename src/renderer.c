#include "renderer.h"
#include "config.h"
#include "file_utils.h"
#include "gl_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint create_shader_program_from_files(const char *vertex_path, const char *fragment_path) {

    // --- SHADER LOADING (once) ---
    char* vs_src = load_file(vertex_path);
    char* fs_src = load_file(fragment_path);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);

    free(vs_src);
    free(fs_src);

    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }

    // Link program
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Program link error: %s\n", log);
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    // Optional: bind attribute locations for ES2
    glBindAttribLocation(program, 0, "aPos");
    glBindAttribLocation(program, 0, "aUV");
    // Shaders can be deleted after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
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


static void init_shader(wayterra_renderer_t *r) {
    r->shader_program = create_shader_program_from_files(
        "shader/vertex.glsl", "shader/fragment.glsl"
    );

    if (!r->shader_program) {
        fprintf(stderr, "Failed to create shader program\n");
        return;
    }

    r->pos_loc = glGetAttribLocation(r->shader_program, "aPos");
    r->uv_loc  = glGetAttribLocation(r->shader_program, "aUV");


    r->loc_renderMode   = glGetUniformLocation(r->shader_program, "uRenderMode");

    r->loc_tileMap      = glGetUniformLocation(r->shader_program, "tileMap");
    r->loc_atlas        = glGetUniformLocation(r->shader_program, "atlasTexture");
    r->loc_player       = glGetUniformLocation(r->shader_program, "playerTexture");
    r->loc_logo         = glGetUniformLocation(r->shader_program, "logoTexture");
    r->loc_windowWidth  = glGetUniformLocation(r->shader_program, "windowWidth");
    r->loc_windowHeight = glGetUniformLocation(r->shader_program, "windowHeight");
    r->loc_tileMapSize  = glGetUniformLocation(r->shader_program, "tileMapSize");
    r->loc_numTiles     = glGetUniformLocation(r->shader_program, "numTiles");
    r->loc_offset       = glGetUniformLocation(r->shader_program, "uOffset");
}

static void init_textures(wayterra_renderer_t *r) {
    r->tileMapTexture        = load_texture("assets/tilemap.png");
    r->atlasTexture          = load_texture("assets/atlas.png");
    r->playerSpriteTexture   = load_texture("assets/player.png");
    r->logoSpriteTexture     = load_texture("assets/logov2.png");
}

static void setup_frame(wayterra_renderer_t *r, int width, int height) {
    glUseProgram(r->shader_program);

    glUniform1f(r->loc_windowWidth,  (float)width);
    glUniform1f(r->loc_windowHeight, (float)height);
    glUniform1f(r->loc_numTiles, NUM_TILES);
    glUniform2f(r->loc_tileMapSize, TILEMAP_WIDTH, TILEMAP_HEIGHT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, r->tileMapTexture);
    glUniform1i(r->loc_tileMap, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, r->atlasTexture);
    glUniform1i(r->loc_atlas, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, r->playerSpriteTexture);
    glUniform1i(r->loc_player, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, r->logoSpriteTexture);
    glUniform1i(r->loc_logo, 3);
}


static void bind_quad(wayterra_renderer_t *r, float *verts) {
    if (r->pos_loc >= 0) {
        glVertexAttribPointer(r->pos_loc, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), verts);
        glEnableVertexAttribArray(r->pos_loc);
    }

    if (r->uv_loc >= 0) {
        glVertexAttribPointer(r->uv_loc, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), verts + 2);
        glEnableVertexAttribArray(r->uv_loc);
    }
}

static void draw_background(wayterra_renderer_t *r) {
    glUniform1i(r->loc_renderMode, 0); // Tell shader to render using the tile map mode
    glUniform2f(r->loc_offset, 0.0f, 0.0f);

    bind_quad(r, r->vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_player(wayterra_renderer_t *r, float x, float y) {
    glUniform1i(r->loc_renderMode, 1); // Tell shader to render using player mode
    glUniform2f(r->loc_offset, x, y);

    bind_quad(r, r->player_vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void draw_logo(wayterra_renderer_t *r, float x, float y) {
    glUniform1i(r->loc_renderMode, 2); // Tell shader to render using player mode
    glUniform2f(r->loc_offset, x, y);

    bind_quad(r, r->logo_vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void initialize_renderer(wayterra_renderer_t *r) {
    init_shader(r);
    init_textures(r);

    // Enable alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    make_quad(r->vertices, -1.0f, -1.0f, 1.0f, 1.0f);   // fullscreen
    make_quad(r->player_vertices, -0.04f, -0.1f, 0.04f, 0.1f);
    make_quad(r->logo_vertices, -0.5f, -0.25f, 0.5f, 0.25f);

    
    r->shader_initialized = true;
}


// TODO 
void renderer_draw_frame(wayterra_renderer_t *r, int width, int height) {
    if (!r->shader_initialized) {
        initialize_renderer(r);
    }

    glViewport(0, 0, width, height);
    setup_frame(r, width, height);


    // TODO update player movement with movement bools and update function
    draw_background(r);
    draw_logo(r, 0.0, -0.3);
}
