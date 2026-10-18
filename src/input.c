#include <stdbool.h>    // bool, true/false
#include <stdint.h>     // uint32_t
#include <stdio.h>      // printf()
#include <stdlib.h>     // calloc(), free(), EXIT_FAILURE
#include <unistd.h>     // fork(), setsid(), execlp(), _exit()
#include <sys/types.h>  // pid_t

#include <wlr/types/wlr_keyboard.h>            // wlr_keyboard_*, wlr_keyboard_key_event
#include <xkbcommon/xkbcommon-keysyms.h>       // XKB_KEY_*
#include <xkbcommon/xkbcommon.h>               // xkb_context/keymap/state, xkb_rule_names

#include "config.h"     // MODIFIER_KEY, USER_KEYMAP, KEY_REPEAT_*
#include "input.h"      // wayterra_keyboard_t, desktop_toplevel_at(); pulls in wayland-server-core.h, wlr_seat.h, wlr_input_device.h
#include "server.h"     // wayterra_server_t, wayterra_output_t, wlr_scene.h, wlr_xdg_shell.h
#include "pointer.h"    // server_new_pointer()
#include "game.h"
struct xkb_rule_names rules = {
    .layout = USER_KEYMAP,
};


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

    // server->cursor_mode = WAYTERRA_CURSOR_MOVE;
	switch (sym) {
	case XKB_KEY_Escape:
		wl_display_terminate(server->wl_display);
		break;
	case XKB_KEY_Return:
		spawn_client("alacritty");   // change to your terminal
		break;
	case XKB_KEY_r:
	case XKB_KEY_R:
		spawn_client("obs");   // change to your terminal
		break;
	case XKB_KEY_b:
	case XKB_KEY_B:
		spawn_client("firefox-developer-edition");   // change to your terminal
		break;
    case XKB_KEY_q:
    case XKB_KEY_Q:
        struct wlr_surface *focused =
            server->seat->keyboard_state.focused_surface;

        if (focused == NULL) {
            break;
        }

        struct wlr_xdg_toplevel *xdg_toplevel =
            wlr_xdg_toplevel_try_from_wlr_surface(focused);

        if (xdg_toplevel == NULL) {
            break;
        }

        // Enable player movement and client movement
        server->game.movement_mode = true;

        wlr_xdg_toplevel_send_close(xdg_toplevel);
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

	/* Track whether MODIFIER_KEY is currently held so other files (e.g. the
	 * pointer button handler) can check it for MODIFIER + drag moving.
	 *
	 * This is the right place for it: wlroots emits the *key* event before it
	 * updates the xkb modifier state, and only emits this modifiers event
	 * after the state has changed. Reading it in the key handler would lag by
	 * one event (Alt's press would read as "not held", its release as "held").
	 */
	server->modifier_held = (wlr_keyboard_get_modifiers(keyboard->wlr_keyboard) & MODIFIER_KEY) != 0;

    /* Ensure this keyboard is the active one for the seat */
    wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

    /* Forward modifier state to the focused client */
    wlr_seat_keyboard_notify_modifiers(
        server->seat,
        &keyboard->wlr_keyboard->modifiers
    );
}

static void wayterra_handle_key(
        struct wl_listener *listener, void *data)
{
    wayterra_keyboard_t *keyboard =
        wl_container_of(listener, keyboard, key);

    wayterra_server_t *server = keyboard->server;

    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = server->seat;

    /* Translate Linux keycode to XKB keycode. */
    uint32_t keycode = event->keycode + 8;

    /* Get keysyms for the active keyboard layout. */
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
        keyboard->wlr_keyboard->xkb_state,
        keycode,
        &syms
    );

    uint32_t modifiers =
        wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

    bool handled = false;

    /*
     * Route input to the game when game movement is enabled.
     */
    if (server->game.movement_mode) {
        for (int i = 0; i < nsyms; i++) {
            if (game_handle_key(
                    &server->game,
                    syms[i],
                    event->state)) {
                handled = true;
            }
        }
    }

    /*
     * Normal compositor keybindings.
     * Only process key presses, not releases.
     */
    if (!handled &&
        (modifiers & MODIFIER_KEY) &&
        event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {

        for (int i = 0; i < nsyms; i++) {
            if (handle_keybinding(
                    keyboard,
                    server,
                    syms[i])) {
                handled = true;
                break;
            }
        }
    }

    /*
     * Forward unhandled events to the focused Wayland client.
     */
    if (!handled) {
        wlr_seat_set_keyboard(
            seat,
            keyboard->wlr_keyboard
        );

        wlr_seat_keyboard_notify_key(
            seat,
            event->time_msec,
            event->keycode,
            event->state
        );
    }
}
void wayterra_new_keyboard(wayterra_server_t *server, struct wlr_input_device *device) {

    
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

    // --- XKB keymap setup ---
    printf("[keyboard] creating xkb context\n");

    struct xkb_context *context =
        xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (!context) {
        printf("[keyboard] ERROR: xkb_context_new failed\n");
        return;
    }
    struct xkb_keymap *keymap =
        xkb_keymap_new_from_names(
                context,
                &rules,
                XKB_KEYMAP_COMPILE_NO_FLAGS
                );

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
    /* Key repeat */
    wlr_keyboard_set_repeat_info(
            wlr_keyboard,
            KEY_REPEAT_RATE,
            KEY_REPEAT_DELAY
            );

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

struct wayterra_toplevel *desktop_toplevel_at(
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
