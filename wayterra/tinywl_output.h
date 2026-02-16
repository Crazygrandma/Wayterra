#ifndef TINYWL_OUTPUT_H
#define TINYWL_OUTPUT_H

#include <wayland-util.h>

struct tinywl_server;
struct wlr_output;
struct wlr_scene_buffer;

struct tinywl_output {
	struct wl_list link;
	struct tinywl_server *server;
	struct wlr_output *wlr_output;
	struct wlr_scene_buffer *blue_background;
	struct wl_listener frame;
	struct wl_listener request_state;
	struct wl_listener destroy;
};

#endif
