#pragma once
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "gl_utils.h"
#include "file_utils.h"

#define TILEMAP_HEIGHT 21.0
#define TILEMAP_WIDTH 21.0
#define NUM_TILES 30.0

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

/* Per-output state */
typedef struct wayterra_output {
    struct wayterra_server *server;
    struct wlr_output *wlr_output;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;

    struct wl_list link; // link into server->outputs list
    
    GLuint shader_program;

    GLuint tileMapTexture;
    GLuint atlasTexture;

    GLint pos_loc;
    GLint uv_loc;

    GLint loc_tileMap;
    GLint loc_atlas;
    GLint loc_windowWidth;
    GLint loc_windowHeight;

    GLint loc_tileMapSize;
    GLint loc_numTiles;

    bool shader_initialized;
    float vertices[24]; // 6 vertices * 4 floats (x, y, u, v)
} wayterra_output_t;

/* Main compositor state */
typedef struct wayterra_server {
    struct wl_display *wl_display;
    struct wl_event_loop *event_loop;   /* Core Wayland event loop */

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wlr_output_layout *output_layout;

    struct wl_list outputs;             /* list of wayterra_output_t */

    struct wlr_scene *scene;            /* Scene graph root */
    struct wlr_scene_output_layout *scene_layout;

    struct wl_listener new_output;      /* listens for backend->events.new_output */

} wayterra_server_t;

// Function declarations
static void setup(wayterra_server_t *server);
static void run(wayterra_server_t *server);
static void cleanup(wayterra_server_t *server);
// static void output_request_state(struct wl_listener *listener, void *data);
// static void output_destroy(struct wl_listener *listener, void *data);
static void output_frame(struct wl_listener *listener, void *data);

static void output_frame(struct wl_listener *listener, void *data) {

    wayterra_output_t *output =
        wl_container_of(listener, output, frame);

    struct wlr_output *wlr_output = output->wlr_output;

    struct wlr_output_state state;
    wlr_output_state_init(&state);

    struct wlr_render_pass *pass =
        wlr_output_begin_render_pass(wlr_output, &state, NULL);

    if (!pass) {
        wlr_output_state_finish(&state);
        return;
    }


    if (!output->shader_initialized) {
        output->shader_program = create_shader_program_from_files(
                "vertex.glsl", "fragment.glsl"
                );
        if (!output->shader_program) {
            fprintf(stderr, "Failed to create shader program\n");
        }

        // Load texture, bind, cleanup  
        output->tileMapTexture = load_texture("assets/tilemap.png");
        output->atlasTexture   = load_texture("assets/atlas.png");

        // Store vertex data in persistent array
        float verts[24] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f,  1.0f, 1.0f, 1.0f,

            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };
        memcpy(output->vertices, verts, sizeof(verts));

        // Cache attribute locations
        output->pos_loc = glGetAttribLocation(output->shader_program, "aPos");
        output->uv_loc  = glGetAttribLocation(output->shader_program, "aUV");

        // Cache uniform locations
        output->loc_tileMap      = glGetUniformLocation(output->shader_program, "tileMap");
        output->loc_atlas        = glGetUniformLocation(output->shader_program, "atlasTexture");
        output->loc_windowWidth  = glGetUniformLocation(output->shader_program, "windowWidth");
        output->loc_windowHeight = glGetUniformLocation(output->shader_program, "windowHeight");
        output->loc_tileMapSize  = glGetUniformLocation(output->shader_program, "tileMapSize");
        output->loc_numTiles     = glGetUniformLocation(output->shader_program, "numTiles");

        output->shader_initialized = true;
    }

    /* Get output size */
    int width, height;
    wlr_output_effective_resolution(wlr_output, &width, &height);
    

    glUseProgram(output->shader_program);

    // Update window-dependent or dynamic uniforms
    glUniform1f(output->loc_windowWidth,  (float)width);
    glUniform1f(output->loc_windowHeight, (float)height);
    glUniform1f(output->loc_numTiles,     NUM_TILES);
    glUniform2f(output->loc_tileMapSize,  TILEMAP_WIDTH, TILEMAP_HEIGHT);

    // Bind textures per frame (texture units can change)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, output->tileMapTexture);   // assign in output struct
    glUniform1i(output->loc_tileMap, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, output->atlasTexture);
    glUniform1i(output->loc_atlas, 1);

    glViewport(0, 0, width, height);

    // Bind vertex attributes to the persistent array
    if (output->pos_loc >= 0) {
        glVertexAttribPointer(output->pos_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), output->vertices);
        glEnableVertexAttribArray(output->pos_loc);
    }

    if (output->uv_loc >= 0) {
        glVertexAttribPointer(output->uv_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), output->vertices + 2);
        glEnableVertexAttribArray(output->uv_loc);
    }

    // Draw full-screen quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    wlr_render_pass_submit(pass);
    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);
}

static void server_new_output(struct wl_listener *listener, void *data) {
    /* Event raised by the backend when a new output is available */
    wayterra_server_t *server =
        wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    /* Configure output to use our allocator and renderer */
    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }

    
    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    /* Allocate per-output state */
    wayterra_output_t *output = calloc(1, sizeof(*output));
    output->wlr_output = wlr_output;
    output->server = server;

    /* Frame listener (for rendering) */
    wl_list_init(&output->frame.link);
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    //
    // /* State request listener */
    // output->request_state.notify = output_request_state;
    // wl_signal_add(&wlr_output->events.request_state, &output->request_state);
    //
    // /* Destroy listener */
    // output->destroy.notify = output_destroy;
    // wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);

    /* Add to output layout */
    struct wlr_output_layout_output *l_output =
        wlr_output_layout_add_auto(server->output_layout, wlr_output);

    struct wlr_scene_output *scene_output =
        wlr_scene_output_create(server->scene, wlr_output);

    wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
}



void setup(wayterra_server_t *server) {
    wlr_log_init(WLR_DEBUG, NULL);


    /* Create Wayland display */
    server->wl_display = wl_display_create();
    if (!server->wl_display) {
        die("Could not create display");
    }

    /* Store the event loop */
    server->event_loop =
        wl_display_get_event_loop(server->wl_display);

    /* Create backend */
    server->backend = wlr_backend_autocreate(
        server->event_loop,
        NULL
    );
    if (!server->backend) {
        die("Could not create backend");
    }

    /* Create renderer */
    server->renderer = wlr_renderer_autocreate(server->backend);
    if (!server->renderer) {
        die("Could not create renderer");
    }

    wlr_renderer_init_wl_display(
        server->renderer,
        server->wl_display
    );

    /* Create allocator */
    server->allocator = wlr_allocator_autocreate(
        server->backend,
        server->renderer
    );
    if (!server->allocator) {
        die("Could not create allocator");
    }

    /* Output layout */
    server->output_layout =
        wlr_output_layout_create(server->wl_display);

    wl_list_init(&server->outputs);

    /* Listen for new outputs */
    server->new_output.notify = server_new_output;
    wl_signal_add(
        &server->backend->events.new_output,
        &server->new_output
    );

    /* Scene graph */
    server->scene = wlr_scene_create();

    server->scene_layout =
        wlr_scene_attach_output_layout(
            server->scene,
            server->output_layout
        );
}

void run(wayterra_server_t *server) {
    /* Create Wayland socket */
    const char *socket =
        wl_display_add_socket_auto(server->wl_display);

    if (!socket)
        die("startup: wl_display_add_socket_auto");

    wlr_log(WLR_INFO, "WAYLAND_DISPLAY=%s", socket);

    setenv("WAYLAND_DISPLAY", socket, 1);

    /* Start backend (enumerates outputs/inputs, takes DRM master, etc.) */
    if (!wlr_backend_start(server->backend))
        die("startup: wlr_backend_start");

    /* Enter event loop (blocks until compositor exits) */
    wl_display_run(server->wl_display);
}

void cleanup(wayterra_server_t *server) {
    /* Remove all connected clients */
    wl_display_destroy_clients(server->wl_display);

    wl_list_remove(&server->new_output.link);
    // /* Destroy scene graph */
    // if (server->scene)
    //     wlr_scene_node_destroy(&server->scene->tree.node);

    /* Destroy allocator */
    if (server->allocator)
        wlr_allocator_destroy(server->allocator);

    if (server->renderer)
        wlr_renderer_destroy(server->renderer);

    /* Destroy backend */
    if (server->backend)
        wlr_backend_destroy(server->backend);

    /* Destroy display (after everything else) */
    if (server->wl_display)
        wl_display_destroy(server->wl_display);
	
}

int main(int argc, char *argv[])
{
	/* Wayland requires XDG_RUNTIME_DIR for creating its communications socket */
	if (!getenv("XDG_RUNTIME_DIR"))
		die("XDG_RUNTIME_DIR must be set");

    wayterra_server_t server = {0};
    
    setup(&server);
    run(&server);
    cleanup(&server);
    return 0;
}
