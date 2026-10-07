#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>     // fork(), setsid(), execlp()
#include <stdlib.h>     // EXIT_FAILURE
#include <sys/types.h>  // pid_t
#include <wayland-util.h>
#include <wlr/types/wlr_keyboard.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "config.h"
#include "clients.h"
#include "input.h"
#include "server.h"
#include "renderer.h"



static void server_new_pointer(wayterra_server_t *server,
		struct wlr_input_device *device) {
	/* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */
	wlr_cursor_attach_input_device(server->cursor, device);
}

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

static void spawn_client(const char *cmd) {
	pid_t pid = fork();
	if (pid == 0) {
		/* child */
		setsid();
		execlp(cmd, cmd, NULL);
		_exit(EXIT_FAILURE);
	}
}

static bool handle_keybinding(wayterra_keyboard_t *keyboard, wayterra_server_t *server, xkb_keysym_t sym) {
	/*
	 * Here we handle compositor keybindings. This is when the compositor is
	 * processing keys, rather than passing them on to the client for its own
	 * processing.
	 *
	 * This function assumes MODIFIER is held down.
	 */
	switch (sym) {
	case XKB_KEY_Escape:
		wl_display_terminate(server->wl_display);
		break;
	case XKB_KEY_Return:
		spawn_client("alacritty");   // change to your terminal
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
    wayterra_output_t *output =
        wl_container_of(server->outputs.next, output, link);

    wayterra_renderer_t *renderer = output->renderer;
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = server->seat;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;

    /* Get a list of keysyms based on the keymap for this keyboard */
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
        keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers =
        wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            if (syms[i] == XKB_KEY_Tab) {
                handled = handle_keybinding(keyboard, server, syms[i]);
            }
        }
    }

    if (!handled && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {

        for (int i = 0; i < nsyms; i++) {
            switch (syms[i]) {
            case XKB_KEY_a:
            case XKB_KEY_A:
                renderer->move_left = true;
                renderer->move_right = false;
                // printf("Move left");
                // handled = true;
                break;

            case XKB_KEY_d:
            case XKB_KEY_D:
                renderer->move_left = false;
                renderer->move_right = true;
                // printf("Move right");
                // handled = true;
                break;
            }
        }
    }

    /* Normal compositor keybindings (only when movement mode is OFF) */
    if (!handled &&
        !keyboard->movement_mode &&
        (modifiers & MODIFIER_KEY) &&
        event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {

        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybinding(keyboard, server, syms[i]);
        }
    }

    /* Pass through to client if not handled */
    if (!handled) {
        wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(
            seat,
            event->time_msec,
            event->keycode,
            event->state
        );
    }
}
void wayterra_new_keyboard(wayterra_server_t *server,
                          struct wlr_input_device *device) {

    
    struct wlr_keyboard *wlr_keyboard =
        wlr_keyboard_from_input_device(device);

    if (!wlr_keyboard) {
        printf("[keyboard] ERROR: wlr_keyboard_from_input_device returned NULL\n");
        return;
    }

    wayterra_keyboard_t *keyboard =
        calloc(1, sizeof(wayterra_keyboard_t));

    if (!keyboard) {
        printf("[keyboard] ERROR: calloc failed\n");
        return;
    }

    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    keyboard->movement_mode = false;

    // --- XKB keymap setup ---
    printf("[keyboard] creating xkb context\n");

    struct xkb_context *context =
        xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (!context) {
        printf("[keyboard] ERROR: xkb_context_new failed\n");
        return;
    }

    // TODO change keymap to value of config.h
    struct xkb_keymap *keymap =
        xkb_keymap_new_from_names(context, NULL,
                                 XKB_KEYMAP_COMPILE_NO_FLAGS);

    if (!keymap) {
        printf("[keyboard] ERROR: xkb_keymap_new_from_names failed\n");
        xkb_context_unref(context);
        return;
    }

    printf("[keyboard] keymap created\n");

    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    // Key repeat config
    printf("[keyboard] setting repeat info\n");
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    // --- Event listeners ---
    printf("[keyboard] setting up listeners\n");

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
    printf("[keyboard] attaching to seat\n");
    wlr_seat_set_keyboard(server->seat, wlr_keyboard);

    // --- Store keyboard ---
    printf("[keyboard] inserting into list\n");
    wl_list_insert(&server->keyboards, &keyboard->link);

    printf("[keyboard] setup complete\n");
}

static void process_cursor_move(wayterra_server_t *server) {
	/* Move the grabbed toplevel to the new position. */
	struct wayterra_toplevel *toplevel = server->grabbed_toplevel;
	wlr_scene_node_set_position(&toplevel->scene_tree->node,
		server->cursor->x - server->grab_x,
		server->cursor->y - server->grab_y);
}

static struct wayterra_toplevel *desktop_toplevel_at(
		wayterra_server_t *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy) {
	/* This returns the topmost node in the scene at the given layout coords.
	 * We only care about surface nodes as we are specifically looking for a
	 * surface in the surface tree of a tinywl_toplevel. */
	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->scene->tree.node, lx, ly, sx, sy);
	if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
		return NULL;
	}
	struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
	struct wlr_scene_surface *scene_surface =
		wlr_scene_surface_try_from_buffer(scene_buffer);
	if (!scene_surface) {
		return NULL;
	}

	*surface = scene_surface->surface;
	/* Find the node corresponding to the tinywl_toplevel at the root of this
	 * surface tree, it is the only one for which we set the data field. */
	struct wlr_scene_tree *tree = node->parent;
	while (tree != NULL && tree->node.data == NULL) {
		tree = tree->node.parent;
	}
	return tree->node.data;
}

void process_cursor_motion(wayterra_server_t *server, uint32_t time) {
	/* If the mode is non-passthrough, delegate to those functions. */
	if (server->cursor_mode == WAYTERRA_CURSOR_MOVE) {
		process_cursor_move(server);
		return;
	} else if (server->cursor_mode == WAYTERRA_CURSOR_RESIZE) {
		// process_cursor_resize(server);
		return;
	}

	/* Otherwise, find the toplevel under the pointer and send the event along. */
	double sx, sy;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *surface = NULL;
	struct wayterra_toplevel *toplevel = desktop_toplevel_at(server,
			server->cursor->x, server->cursor->y, &surface, &sx, &sy);
	if (!toplevel) {
		/* If there's no toplevel under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any toplevels. */
		wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
	}
	if (surface) {
		/*
		 * Send pointer enter and motion events.
		 *
		 * The enter event gives the surface "pointer focus", which is distinct
		 * from keyboard focus. You get pointer focus by moving the pointer over
		 * a window.
		 *
		 * Note that wlroots will avoid sending duplicate enter/motion events if
		 * the surface has already has pointer focus or if the client is already
		 * aware of the coordinates passed.
		 */
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {
		/* Clear pointer focus so future button events and such are not sent to
		 * the last client to have the cursor over it. */
		wlr_seat_pointer_clear_focus(seat);
	}
}

static void reset_cursor_mode(wayterra_server_t *server) {
	/* Reset the cursor mode to passthrough. */
	server->cursor_mode = WAYTERRA_CURSOR_PASSTHROUGH;
	server->grabbed_toplevel = NULL;
}

void server_cursor_button(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a button
	 * event. */
	wayterra_server_t *server =
		wl_container_of(listener, server, cursor_button);
	struct wlr_pointer_button_event *event = data;
	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(server->seat,
			event->time_msec, event->button, event->state);
	if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
		/* If you released any buttons, we exit interactive move/resize mode. */
		reset_cursor_mode(server);
	} else {
		/* Focus that client if the button was _pressed_ */
		double sx, sy;
		struct wlr_surface *surface = NULL;
		struct wayterra_toplevel *toplevel = desktop_toplevel_at(server,
				server->cursor->x, server->cursor->y, &surface, &sx, &sy);
		focus_toplevel(toplevel);
	}
}
void server_cursor_motion(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a _relative_
	 * pointer motion event (i.e. a delta) */
	wayterra_server_t *server =
		wl_container_of(listener, server, cursor_motion);
	struct wlr_pointer_motion_event *event = data;
	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(server->cursor, &event->pointer->base,
			event->delta_x, event->delta_y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_motion_absolute(
		struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an _absolute_
	 * motion event, from 0..1 on each axis. This happens, for example, when
	 * wlroots is running under a Wayland window rather than KMS+DRM, and you
	 * move the mouse over the window. You could enter the window from any edge,
	 * so we have to warp the mouse there. There is also some hardware which
	 * emits these events. */
	wayterra_server_t *server =
		wl_container_of(listener, server, cursor_motion_absolute);
	struct wlr_pointer_motion_absolute_event *event = data;
	wlr_cursor_warp_absolute(server->cursor, &event->pointer->base, event->x,
		event->y);
	process_cursor_motion(server, event->time_msec);
}

// TODO Fix and refactor input handling
void server_new_input(struct wl_listener *listener, void *data) {
    // Get pointer to the server struct for this listener 
    wayterra_server_t *server =
        wl_container_of(listener, server, new_input);

    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        wayterra_new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        server_new_pointer(server, device);
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
