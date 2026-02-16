#ifndef TINYWL_XDG_H
#define TINYWL_XDG_H

#include <wayland-util.h>

struct tinywl_server;
struct wlr_xdg_toplevel;
struct wlr_scene_tree;
struct wlr_xdg_popup;

struct tinywl_toplevel {
	struct wl_list link;
	struct tinywl_server *server;
	struct wlr_xdg_toplevel *xdg_toplevel;
	struct wlr_scene_tree *scene_tree;
	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener commit;
	struct wl_listener destroy;
	struct wl_listener request_move;
	struct wl_listener request_resize;
	struct wl_listener request_maximize;
	struct wl_listener request_fullscreen;
};

struct tinywl_popup {
	struct wlr_xdg_popup *xdg_popup;
	struct wl_listener commit;
	struct wl_listener destroy;
};

#endif
