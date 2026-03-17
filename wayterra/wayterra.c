
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/libinput.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_output.h>
#include "util.h"

#include <wlr/util/log.h>

/* Main compositor state */
typedef struct wayterra_server {
    struct wl_display *wl_display;

    struct wl_event_loop *event_loop;   /* Core Wayland event loop */

    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    struct wlr_output_layout *output_layout;

    struct wl_list outputs;             /* Active outputs */
    struct wl_listener new_output;       /* Backend new_output listener */

    struct wlr_scene *scene;            /* Scene graph root */
    struct wlr_scene_output_layout *scene_layout;

} wayterra_server_t;


// Function delarations
static void setup(wayterra_server_t *server);
static void run(wayterra_server_t *server);
static void cleanup(wayterra_server_t *server);
static void handle_new_output(struct wl_listener* listener, void *data);


static void handle_new_output(struct wl_listener *listener, void *data) {
    struct wayterra_server *server =
        wl_container_of(listener, server, new_output);

    struct wlr_output *wlr_output = data;

    /* Initialize rendering for this output */
    if (!wlr_output_init_render(
            wlr_output,
            server->allocator,
            server->renderer)) {
        wlr_log(WLR_ERROR,
            "Failed to init render for output %s",
            wlr_output->name);
        return;
    }

    /* Prepare output state */
    struct wlr_output_state state;
    wlr_output_state_init(&state);

    wlr_output_state_set_enabled(&state, true);

    /* Set preferred mode */
    struct wlr_output_mode *mode =
        wlr_output_preferred_mode(wlr_output);

    if (mode) {
        wlr_output_state_set_mode(&state, mode);
    }

    /* Commit configuration */
    if (!wlr_output_commit_state(wlr_output, &state)) {
        wlr_log(WLR_ERROR,
            "Failed to commit output state for %s",
            wlr_output->name);
        wlr_output_state_finish(&state);
        return;
    }

    wlr_output_state_finish(&state);

    /* Add output to layout (auto-position) */
    wlr_output_layout_add_auto(
        server->output_layout,
        wlr_output
    );

    /* Create scene output (required for rendering) */
    wlr_scene_output_create(
        server->scene,
        wlr_output
    );
  /* Get output resolution */
    int width, height;
    wlr_output_effective_resolution(
        wlr_output,
        &width,
        &height
    );

    /* Create background rectangle as child of scene root */
    struct wlr_scene_rect *bg =
        wlr_scene_rect_create(
            server->scene,
            width,
            height,
            (float[4]){0.1f, 0.1f, 0.1f, 1.0f}
        );

    /* Ensure it is positioned at (0,0) */
    wlr_scene_node_set_position(
        &bg->node,
        0,
        0
    );
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
    server->new_output.notify = handle_new_output;
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

    /* Destroy scene graph */
    if (server->scene)
        wlr_scene_node_destroy(&server->scene->tree.node);

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
