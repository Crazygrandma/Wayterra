#include <stdlib.h>

#include <wayland-server-protocol.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_seat.h>

#include "error.h"
#include "output.h"
#include "server.h"

static void wayterra_handle_keyboard_destroy(struct wl_listener *listener, void *data) {
	/* This event is raised by the keyboard base wlr_input_device to signal
	 * the destruction of the wlr_keyboard. It will no longer receive events
	 * and should be destroyed.
	 */
    wayterra_keyboard_t *keyboard =
		wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->link);
	free(keyboard);
}

static bool handle_keybinding(wayterra_server_t *server, xkb_keysym_t sym) {
	/*
	 * Here we handle compositor keybindings. This is when the compositor is
	 * processing keys, rather than passing them on to the client for its own
	 * processing.
	 *
	 * This function assumes Alt is held down.
	 */
	switch (sym) {
	case XKB_KEY_Escape:
		wl_display_terminate(server->wl_display);
		break;
	default:
		return false;
	}
	return true;
}

void wayterra_handle_modifiers(struct wl_listener *listener, void *data) {
    wayterra_keyboard_t *keyboard =
        wl_container_of(listener, keyboard, modifiers);

	wayterra_server_t *server = keyboard->server;

    /* Ensure this keyboard is the active one for the seat */
    wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

    /* Forward modifier state to the focused client */
    wlr_seat_keyboard_notify_modifiers(
        server->seat,
        &keyboard->wlr_keyboard->modifiers
    );
}

static void wayterra_handle_key(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a key is pressed or released. */
	wayterra_keyboard_t *keyboard =
		wl_container_of(listener, keyboard, key);
	wayterra_server_t *server = keyboard->server;
	struct wlr_keyboard_key_event *event = data;
	struct wlr_seat *seat = server->seat;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->wlr_keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
	if ((modifiers & WLR_MODIFIER_ALT) &&
			event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		/* If alt is held down and this button was _pressed_, we attempt to
		 * process it as a compositor keybinding. */
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, syms[i]);
		}
	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}

void wayterra_new_keyboard(wayterra_server_t *server,
                          struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard =
        wlr_keyboard_from_input_device(device);

    wayterra_keyboard_t *keyboard =
        calloc(1, sizeof(wayterra_keyboard_t));
    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    // --- XKB keymap setup ---
    struct xkb_context *context =
        xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    struct xkb_keymap *keymap =
        xkb_keymap_new_from_names(context, NULL,
                                 XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    // Key repeat config
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    // --- Event listeners ---
    keyboard->modifiers.notify = wayterra_handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers,
                  &keyboard->modifiers);

    keyboard->key.notify = wayterra_handle_key;
    wl_signal_add(&wlr_keyboard->events.key,
                  &keyboard->key);

    keyboard->destroy.notify = wayterra_handle_keyboard_destroy;
    wl_signal_add(&device->events.destroy,
                  &keyboard->destroy);

    // --- Attach to seat ---
    wlr_seat_set_keyboard(server->seat, wlr_keyboard);

    // --- Store keyboard ---
    wl_list_insert(&server->keyboards, &keyboard->link);
}

void server_new_input(struct wl_listener *listener, void *data) {
    wayterra_server_t *server =
        wl_container_of(listener, server, new_input);

    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        wayterra_new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        // handle_new_pointer(server, device);
        break;
    default:
        break;
    }

    // Update seat capabilities
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;

    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    wlr_seat_set_capabilities(server->seat, caps);
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

	wl_list_remove(&server->new_input.link);

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
