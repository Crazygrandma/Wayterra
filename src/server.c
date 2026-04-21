#include <stdlib.h>

#include <wayland-server-protocol.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_seat.h>

#include "error.h"
#include "output.h"
#include "server.h"
#include "input.h"
#include "clients.h"


void setup(wayterra_server_t *server) {
    // Turn on logging so we can see what goes wrong
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

	/* This creates some hands-off wlroots interfaces. The compositor is
	 * necessary for clients to allocate surfaces, the subcompositor allows to
	 * assign the role of subsurfaces to surfaces and the data device manager
	 * handles the clipboard. Each of these wlroots interfaces has room for you
	 * to dig your fingers in and play with their behavior if you want. Note that
	 * the clients cannot set the selection directly without compositor approval,
	 * see the handling of the request_set_selection event below.*/
	wlr_compositor_create(server->wl_display, 5, server->renderer);
	wlr_subcompositor_create(server->wl_display);
	wlr_data_device_manager_create(server->wl_display);
   

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


    // Listen for new input devices such as a mouse and keyboard
	wl_list_init(&server->keyboards);
	server->new_input.notify = server_new_input;
	wl_signal_add(&server->backend->events.new_input, &server->new_input);
    server->seat = wlr_seat_create(server->wl_display, "seat0");

    /* Scene graph */
    server->scene = wlr_scene_create();

    server->scene_layout =
        wlr_scene_attach_output_layout(
            server->scene,
            server->output_layout
        );

	/* Set up xdg-shell version 3. The xdg-shell is a Wayland protocol which is
	 * used for application windows. For more detail on shells, refer to
	 * https://drewdevault.com/2018/07/29/Wayland-shells.html.
	 */
	wl_list_init(&server->toplevels);
	server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
	server->new_xdg_toplevel.notify = server_new_xdg_toplevel;
	wl_signal_add(&server->xdg_shell->events.new_toplevel, &server->new_xdg_toplevel);
	// server->new_xdg_popup.notify = server_new_xdg_popup;
 //    wl_signal_add(&server->xdg_shell->events.new_popup, &server->new_xdg_popup);
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

	wl_list_remove(&server->new_xdg_toplevel.link);

    wl_list_remove(&server->new_input.link);

    wl_list_remove(&server->new_output.link);
    // /* Destroy scene graph */
    //
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


    /* Destroy display (after everything else) */
    if (server->wl_display)
        wl_display_destroy(server->wl_display);
	
}
