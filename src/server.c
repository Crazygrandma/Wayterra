#include <stdlib.h>

#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>

#include "error.h"
#include "output.h"
#include "server.h"

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


    /* Destroy display (after everything else) */
    if (server->wl_display)
        wl_display_destroy(server->wl_display);
	
}
