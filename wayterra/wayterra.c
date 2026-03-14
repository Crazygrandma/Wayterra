
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

struct wayterra_server{
    struct wl_display *wl_display;
    struct wlr_session *session;
    struct wlr_backend *backend;
    struct wlr_allocator *allocator;
    struct wlr_renderer *renderer;
    struct wlr_scene *scene;
	struct wlr_scene_output_layout *scene_layout;
	struct wlr_output_layout *output_layout;

    struct wl_list outputs;
	struct wl_listener new_output;
};
// Function delarations
static void setup(void);
static void run(void); 
static void cleanup(void);
static void handle_new_output(struct wl_listener* listener, void *data);



static void handle_new_output(struct wl_listener *listener, void *data){

	struct wayterra_server *server =
		wl_container_of(listener, server, new_output);
	struct wlr_output *wlr_output = data;

    if(!wlr_output_init_render(wlr_output, server->allocator, server->renderer)){
        wlr_log(WLR_ERROR, "Failed to init render for output %s", wlr_output->name);
        return;
    }

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    
	struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
	if (mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}
	
	
	/* Atomically applies the new output state. */
	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

}

void setup(void){
	wlr_log_init(WLR_DEBUG, NULL);

    struct wayterra_server server = {0};
    server.wl_display = wl_display_create();
    if (!server.wl_display) {
        die("Could not create display");
    }
    
	server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
	if (server.backend == NULL) {
		die("couldn't create backend");
    }


	server.renderer = wlr_renderer_autocreate(server.backend);
	if (server.renderer == NULL) {
		die("couldn't create renderer");
	}

	wlr_renderer_init_wl_display(server.renderer, server.wl_display);

	server.allocator = wlr_allocator_autocreate(server.backend,
		server.renderer);
	if (server.allocator == NULL) {
		die("couldn't create allocator");
	}
	
	server.output_layout = wlr_output_layout_create(server.wl_display);
    
	/* Configure a listener to be notified when new outputs are available on the
	 * backend. */
	wl_list_init(&server.outputs);
	server.new_output.notify = handle_new_output;
	wl_signal_add(&server.backend->events.new_output, &server.new_output);


	/* Create a scene graph. This is a wlroots abstraction that handles all
	 * rendering and damage tracking. All the compositor author needs to do
	 * is add things that should be rendered to the scene graph at the proper
	 * positions and then call wlr_scene_output_commit() to render a frame if
	 * necessary.
	 */
	server.scene = wlr_scene_create();
	server.scene_layout = wlr_scene_attach_output_layout(server.scene, server.output_layout);
}

void run(void){
	/* Add a Unix socket to the Wayland display.*/
    struct wayterra_server server;
	const char *socket = wl_display_add_socket_auto(server.wl_display);
    wlr_log(WLR_INFO, "WAYLAND_DISPLAY=%s", socket);
	if (!socket)
		die("startup: display_add_socket_auto");
	setenv("WAYLAND_DISPLAY", socket, 1);

	/* Start the backend. This will enumerate outputs and inputs, become the DRM
	 * master, etc */
	if (!wlr_backend_start(server.backend))
		die("startup: backend_start");

	/* Run the Wayland event loop. This does not return until you exit the
	 * compositor. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */
	wl_display_run(server.wl_display);
}

void cleanup(void){
    struct wayterra_server server;
	

    /* Once wl_display_run returns, we destroy all clients then shut down the
	 * server. */
	wl_display_destroy_clients(server.wl_display);

    

    wlr_scene_node_destroy(&server.scene->tree.node);
    /* If it's not destroyed manually, it will cause a use-after-free of wlr_seat.
	 * Destroy it until it's fixed on the wlroots side */
	wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);
	

}

int main(int argc, char *argv[])
{
	/* Wayland requires XDG_RUNTIME_DIR for creating its communications socket */
	if (!getenv("XDG_RUNTIME_DIR"))
		die("XDG_RUNTIME_DIR must be set");
    setup();
    run();
    cleanup();
    return 0;
}
