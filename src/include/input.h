#ifndef WAYTERRA_INPUT_H
#define WAYTERRA_INPUT_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_cursor.h>


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


/* Defined in input.c, referenced by server.c: must have external linkage. */
void server_cursor_button(struct wl_listener *listener, void *data);
void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
void server_cursor_motion(struct wl_listener *listener, void *data);
void process_cursor_motion(wayterra_server_t *server, uint32_t time);
void wayterra_handle_modifiers(struct wl_listener *listener, void *data);
void server_new_input(struct wl_listener *listener, void *data);

#endif /* WAYTERRA_INPUT_H */
