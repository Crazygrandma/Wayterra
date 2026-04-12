#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>


typedef struct wayterra_server wayterra_server_t;

typedef struct wayterra_keyboard {
	wayterra_server_t *server;
	struct wlr_keyboard *wlr_keyboard;

	struct wl_listener modifiers;
	struct wl_listener key;
	struct wl_listener destroy;

    struct wl_list link;

    bool movement_mode;
} wayterra_keyboard_t;


void wayterra_handle_modifiers(struct wl_listener *listener, void *data);
static bool handle_keybinding(wayterra_keyboard_t *keyboard,wayterra_server_t *server, xkb_keysym_t sym);
static void wayterra_handle_keyboard_destroy(struct wl_listener *listener, void *data);
void server_new_input(struct wl_listener *listener, void *data);
