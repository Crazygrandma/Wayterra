#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <wayland-util.h>
#include <wlr/types/wlr_keyboard.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "config.h"
#include "input.h"
#include "server.h"
#include "renderer.h"

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


// TODO use game state
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
    case XKB_KEY_Tab:
        keyboard->movement_mode = !keyboard->movement_mode;
        printf("Toggle movement mode");
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

    /* Always allow Tab toggle (so you don't get stuck in movement mode) */
    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            if (syms[i] == XKB_KEY_Tab) {
                handled = handle_keybinding(keyboard, server, syms[i]);
            }
        }
    }

    // TODO Call renderer input handler
    /* Movement mode: WASD without modifier */
    // if (!handled &&
    //     keyboard->movement_mode &&
    //     event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    //
    //     for (int i = 0; i < nsyms; i++) {
    //         switch (syms[i]) {
    //         case XKB_KEY_w:
    //         case XKB_KEY_W:
    //             update_player_pos(renderer, 0, -PLAYER_SPEED);
    //             handled = true;
    //             break;
    //
    //         case XKB_KEY_s:
    //         case XKB_KEY_S:
    //             update_player_pos(renderer, 0, PLAYER_SPEED);
    //             handled = true;
    //             break;
    //
    //         case XKB_KEY_a:
    //         case XKB_KEY_A:
    //             update_player_pos(renderer, -PLAYER_SPEED, 0);
    //             handled = true;
    //             break;
    //
    //         case XKB_KEY_d:
    //         case XKB_KEY_D:
    //             update_player_pos(renderer, PLAYER_SPEED, 0);
    //             handled = true;
    //             break;
    //         }
    //     }
    // }

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
