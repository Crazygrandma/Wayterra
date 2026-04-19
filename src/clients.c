#include "clients.h"
#include <wayland-util.h>
#include "server.h"
#include <stdlib.h>

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>

void server_new_xdg_toplevel(struct wl_listener *listener, void *data) {
	/* This event is raised when a client creates a new toplevel (application window). */
	wayterra_server_t *server = wl_container_of(listener, server, new_xdg_toplevel);
	struct wlr_xdg_toplevel *xdg_toplevel = data;

	/* Allocate a tinywl_toplevel for this surface */
	struct wayterra_toplevel *toplevel = calloc(1, sizeof(*toplevel));
	toplevel->server = server;
	toplevel->xdg_toplevel = xdg_toplevel;
	toplevel->scene_tree =
		wlr_scene_xdg_surface_create(&toplevel->server->scene->tree, xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	xdg_toplevel->base->data = toplevel->scene_tree;

	/* Listen to the various events it can emit */
	// toplevel->map.notify = xdg_toplevel_map;
	// wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
	// toplevel->unmap.notify = xdg_toplevel_unmap;
	// wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
	// toplevel->commit.notify = xdg_toplevel_commit;
	// wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);
	//
	// toplevel->destroy.notify = xdg_toplevel_destroy;
	// wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);
	//
	// /* cotd */
	// toplevel->request_move.notify = xdg_toplevel_request_move;
	// wl_signal_add(&xdg_toplevel->events.request_move, &toplevel->request_move);
	// toplevel->request_resize.notify = xdg_toplevel_request_resize;
	// wl_signal_add(&xdg_toplevel->events.request_resize, &toplevel->request_resize);
	// toplevel->request_maximize.notify = xdg_toplevel_request_maximize;
	// wl_signal_add(&xdg_toplevel->events.request_maximize, &toplevel->request_maximize);
	// toplevel->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
	// wl_signal_add(&xdg_toplevel->events.request_fullscreen, &toplevel->request_fullscreen);
}



// void server_new_xdg_popup(struct wl_listener *listener, void *data) {
// 	/* This event is raised when a client creates a new popup. */
// 	struct wlr_xdg_popup *xdg_popup = data;
//
// 	struct tinywl_popup *popup = calloc(1, sizeof(*popup));
// 	popup->xdg_popup = xdg_popup;
//
// 	/* We must add xdg popups to the scene graph so they get rendered. The
// 	 * wlroots scene graph provides a helper for this, but to use it we must
// 	 * provide the proper parent scene node of the xdg popup. To enable this,
// 	 * we always set the user data field of xdg_surfaces to the corresponding
// 	 * scene node. */
// 	struct wlr_xdg_surface *parent = wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
// 	assert(parent != NULL);
// 	struct wlr_scene_tree *parent_tree = parent->data;
// 	xdg_popup->base->data = wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);
//
// 	popup->commit.notify = xdg_popup_commit;
// 	wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);
//
// 	popup->destroy.notify = xdg_popup_destroy;
// 	wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
// }
