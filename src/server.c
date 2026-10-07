#include <stdlib.h>

#include <wayland-server-protocol.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/render/gles2.h>
#include "error.h"
#include "output.h"
#include "server.h"
#include "input.h"
#include "clients.h"


void setup(wayterra_server_t *server)
{
    /* Logging */
    wlr_log_init(WLR_INFO, NULL);

    /* Wayland display */
    server->wl_display = wl_display_create();
    if (!server->wl_display) {
        die("Could not create display");
    }

    server->event_loop =
        wl_display_get_event_loop(server->wl_display);

    /* Backend */
    server->backend =
        wlr_backend_autocreate(server->event_loop, NULL);
    if (!server->backend) {
        die("Could not create backend");
    }

    /* Renderer */
    server->renderer =
        wlr_renderer_autocreate(server->backend);
    if (!server->renderer) {
        die("Could not create renderer");
    }

    if (!wlr_renderer_is_gles2(server->renderer)) {
        wlr_log(WLR_ERROR,
            "Need a GLES2 renderer for game rendering");
        return;
    }

    if (!wlr_renderer_init_wl_display(
            server->renderer,
            server->wl_display)) {
        die("Could not initialise wl_shm, linux_dmabuf");
    }

    /* Allocator */
    server->allocator =
        wlr_allocator_autocreate(
            server->backend,
            server->renderer
        );

    if (!server->allocator) {
        die("Could not create allocator");
    }

    /* Wayland globals */
    wlr_compositor_create(
        server->wl_display,
        5,
        server->renderer
    );

    wlr_subcompositor_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);


    
    /* Output */
    server->output_layout =
        wlr_output_layout_create(server->wl_display);

    wl_list_init(&server->outputs);

    server->new_output.notify = server_new_output;

    wl_signal_add(
        &server->backend->events.new_output,
        &server->new_output
    );

    
    /* Input */
    wl_list_init(&server->keyboards);

    server->new_input.notify = server_new_input;

    wl_signal_add(
        &server->backend->events.new_input,
        &server->new_input
    );

    server->seat =
        wlr_seat_create(server->wl_display, "seat0");


	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	server->cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(server->cursor, server->output_layout);

	/* Creates an xcursor manager, another wlroots utility which loads up
	 * Xcursor themes to source cursor images from and makes sure that cursor
	 * images are available at all scale factors on the screen (necessary for
	 * HiDPI support). */
	server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

	server->cursor_mode = WAYTERRA_CURSOR_PASSTHROUGH;
	server->cursor_motion.notify = server_cursor_motion;
	wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);
	server->cursor_motion_absolute.notify = server_cursor_motion_absolute;
	wl_signal_add(&server->cursor->events.motion_absolute,
			&server->cursor_motion_absolute);
	server->cursor_button.notify = server_cursor_button;
	wl_signal_add(&server->cursor->events.button, &server->cursor_button);



    /* Scene graph */
    server->scene = wlr_scene_create();

    server->scene_layout =
        wlr_scene_attach_output_layout(
            server->scene,
            server->output_layout
        );

    /* XDG shell */
    wl_list_init(&server->toplevels);

    server->xdg_shell =
        wlr_xdg_shell_create(server->wl_display, 3);

    server->new_xdg_toplevel.notify =
        server_new_xdg_toplevel;

    wl_signal_add(
        &server->xdg_shell->events.new_toplevel,
        &server->new_xdg_toplevel
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

	wl_list_remove(&server->cursor_motion.link);
	wl_list_remove(&server->cursor_motion_absolute.link);
    wl_list_remove(&server->new_xdg_toplevel.link);
    wl_list_remove(&server->new_input.link);
    wl_list_remove(&server->new_output.link);

    /* Destroy per-output resources */
    wayterra_output_t *output, *tmp;
    wl_list_for_each_safe(output, tmp, &server->outputs, link) {
        if (output->gameframebuffer) {
            wlr_buffer_drop(output->gameframebuffer);
            output->gameframebuffer = NULL;
        }

        /* Detach from the outputs first: the backend destroys them
         * below, which would otherwise invoke listeners pointing at
         * already freed memory. output_destroy() does the same when an
         * output disappears at runtime. */
        wl_list_remove(&output->frame.link);
        wl_list_remove(&output->request_state.link);
        wl_list_remove(&output->destroy.link);

        wl_list_remove(&output->link);
        free(output);
    }

    /* Destroy scene graph */
    if (server->scene)
        wlr_scene_node_destroy(&server->scene->tree.node);

    /* Destroy allocator */
    if (server->allocator)
        wlr_allocator_destroy(server->allocator);

    if (server->renderer)
        wlr_renderer_destroy(server->renderer);

    /* Destroy backend */
    if (server->backend)
        wlr_backend_destroy(server->backend);

    /* Destroy display */
    if (server->wl_display)
        wl_display_destroy(server->wl_display);
}
