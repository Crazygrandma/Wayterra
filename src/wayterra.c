#pragma once
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_cursor.h>
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

#include <time.h>


typedef struct wayterra_renderer{

    GLuint shader_program;

    GLuint tileMapTexture;
    GLuint atlasTexture;
    GLuint playerSpriteTexture;

    GLint pos_loc;
    GLint uv_loc;

    GLint loc_renderMode;
    
    GLint loc_offset;
    GLint loc_tileMap;
    GLint loc_atlas;
    GLint loc_player;
    GLint loc_windowWidth;
    GLint loc_windowHeight;

    GLint loc_tileMapSize;
    GLint loc_numTiles;

    bool shader_initialized;
    float vertices[24]; // 6 vertices * 4 floats (x, y, u, v)

    float player_vertices[24]; // same format: x, y, u, v
} wayterra_renderer_t;


/* Per-output state */
typedef struct wayterra_output {
    struct wayterra_server *server;
    struct wlr_output *wlr_output;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;

    struct wl_list link; // link into server->outputs list
  
    wayterra_renderer_t *renderer;  // <--- this line
} wayterra_output_t;



/* Main compositor state */
typedef struct wayterra_server {
    struct wl_display *wl_display;
    struct wl_event_loop *event_loop;   /* Core Wayland event loop */

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    // NOT BEING USED
    struct wlr_scene *scene;            /* Scene graph root */
    struct wlr_scene_output_layout *scene_layout;
        

	struct wlr_cursor *cursor;
    
    struct wlr_output_layout *output_layout;
    struct wl_list outputs;             /* list of wayterra_output_t */
    struct wl_listener new_output;      /* listens for backend->events.new_output */

} wayterra_server_t;



float get_time_seconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (float)ts.tv_sec + (float)ts.tv_nsec / 1e9f;
}

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

// Function declarations
static void setup(wayterra_server_t *server);
static void run(wayterra_server_t *server);
static void cleanup(wayterra_server_t *server);
// static void output_request_state(struct wl_listener *listener, void *data);
// static void output_destroy(struct wl_listener *listener, void *data);
static void output_frame(struct wl_listener *listener, void *data);



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
        "vertex.glsl", "fragment.glsl"
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
}

void initialize_renderer(wayterra_renderer_t *r) {
    init_shader(r);
    init_textures(r);

    // Enable alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    make_quad(r->vertices, -1.0f, -1.0f, 1.0f, 1.0f);   // fullscreen
    make_quad(r->player_vertices, -0.1f, -0.1f, 0.1f, 0.1f);

    r->shader_initialized = true;
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

static void output_frame(struct wl_listener *listener, void *data) {
    wayterra_output_t *output =
        wl_container_of(listener, output, frame);

    struct wlr_output *wlr_output = output->wlr_output;
    wayterra_renderer_t *r = output->renderer;

    struct wlr_output_state state;
    wlr_output_state_init(&state);

    struct wlr_render_pass *pass =
        wlr_output_begin_render_pass(wlr_output, &state, NULL);

    if (!pass) {
        wlr_output_state_finish(&state);
        return;
    }

    if (!r->shader_initialized) {
        initialize_renderer(r);
    }

    int width, height;
    wlr_output_effective_resolution(wlr_output, &width, &height);

    glViewport(0, 0, width, height);

    setup_frame(r, width, height);


    draw_background(r);
    draw_player(r, 0.0, 0.0);

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

    /* Allocate renderer for this output */
    output->renderer = calloc(1, sizeof(*output->renderer));

    /* Optionally: initialize vertices array to zero (calloc already does this) */
    output->renderer->shader_initialized = false;

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

    // TODO Add additional listeners

    /* Scene graph */
    server->scene = wlr_scene_create();

    server->scene_layout =
        wlr_scene_attach_output_layout(
            server->scene,
            server->output_layout
        );
	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	// server->cursor = wlr_cursor_create();
	// wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
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

    wlr_cursor_destroy(server->cursor);

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
