
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/libinput.h>
#include <wlr/render/allocator.h>

#include "util.h"

#include <wlr/util/log.h>

static struct wl_display *wayterra_display;
static struct wl_event_loop *event_loop;
static struct wlr_session *session;
static struct wlr_backend *backend;
// Function delarations
static void setup(void);
static void run(void);
static void cleanup(void);



void setup(void){
	wlr_log_init(WLR_DEBUG, NULL);
    
    wayterra_display = wl_display_create();
    if (!wayterra_display) {
        die("Could not create display");
    }
	event_loop = wl_display_get_event_loop(wayterra_display);
    
	/* The backend is a wlroots feature which abstracts the underlying input and
	 * output hardware. The autocreate option will choose the most suitable
	 * backend based on the current environment, such as opening an X11 window
	 * if an X11 server is running. */
	if (!(backend = wlr_backend_autocreate(event_loop, &session))){
		die("couldn't create backend");
    }
}

void run(void){
	/* Add a Unix socket to the Wayland display. */
	const char *socket = wl_display_add_socket_auto(wayterra_display);
    wlr_log(WLR_INFO, "WAYLAND_DISPLAY=%s", socket);
	if (!socket)
		die("startup: display_add_socket_auto");
	setenv("WAYLAND_DISPLAY", socket, 1);
	/* Start the backend. This will enumerate outputs and inputs, become the DRM
	 * master, etc */
	if (!wlr_backend_start(backend))
		die("startup: backend_start");

	/* Run the Wayland event loop. This does not return until you exit the
	 * compositor. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */
	wl_display_run(wayterra_display);
}

void cleanup(void){
	/* If it's not destroyed manually, it will cause a use-after-free of wlr_seat.
	 * Destroy it until it's fixed on the wlroots side */
	wlr_backend_destroy(backend);
    wl_display_destroy(wayterra_display);
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
