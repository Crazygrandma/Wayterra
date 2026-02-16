#ifndef TINYWL_INPUT_H
#define TINYWL_INPUT_H

#include <wayland-util.h>

struct tinywl_server;
struct wlr_keyboard;

struct tinywl_keyboard {
	struct wl_list link;
	struct tinywl_server *server;
	struct wlr_keyboard *wlr_keyboard;

	struct wl_listener modifiers;
	struct wl_listener key;
	struct wl_listener destroy;
};

#endif
