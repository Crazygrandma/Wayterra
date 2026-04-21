#include "clients.h"
#include <wayland-util.h>
#include "server.h"
#include <stdlib.h>

#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>

// TODO split toplevel, popup into files

static void focus_toplevel(struct wayterra_toplevel *toplevel) {
	/* Note: this function only deals with keyboard focus. */
	if (toplevel == NULL) {
		return;
	}
	wayterra_server_t *server = toplevel->server;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	struct wlr_surface *surface = toplevel->xdg_toplevel->base->surface;
	if (prev_surface == surface) {
		/* Don't re-focus an already focused surface. */
		return;
	}
	if (prev_surface) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		struct wlr_xdg_toplevel *prev_toplevel =
			wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
		if (prev_toplevel != NULL) {
			wlr_xdg_toplevel_set_activated(prev_toplevel, false);
		}
	}
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
	/* Move the toplevel to the front */
	wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
	wl_list_remove(&toplevel->link);
	wl_list_insert(&server->toplevels, &toplevel->link);
	/* Activate the new surface */
	wlr_xdg_toplevel_set_activated(toplevel->xdg_toplevel, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	if (keyboard != NULL) {
		wlr_seat_keyboard_notify_enter(seat, surface,
			keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
	}
}

void xdg_toplevel_map(struct wl_listener *listener, void *data) {
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct wayterra_toplevel *toplevel = wl_container_of(listener, toplevel, map);

	wl_list_insert(&toplevel->server->toplevels, &toplevel->link);

	focus_toplevel(toplevel);
}

void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct wayterra_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

	// /* Reset the cursor mode if the grabbed toplevel was unmapped. */
	// if (toplevel == toplevel->server->grabbed_toplevel) {
	// 	reset_cursor_mode(toplevel->server);
	// }

	wl_list_remove(&toplevel->link);
}

void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
	/* Called when a new surface state is committed. */
	struct wayterra_toplevel *toplevel = wl_container_of(listener, toplevel, commit);

	if (toplevel->xdg_toplevel->base->initial_commit) {
		/* When an xdg_surface performs an initial commit, the compositor must
		 * reply with a configure so the client can map the surface. tinywl
		 * configures the xdg_toplevel with 0,0 size to let the client pick the
		 * dimensions itself. */
		wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
	}
}


void server_new_xdg_toplevel(struct wl_listener *listener, void *data) {
	/* This event is raised when a client creates a new toplevel (application window). */
	wayterra_server_t *server = wl_container_of(listener, server, new_xdg_toplevel);
	struct wlr_xdg_toplevel *xdg_toplevel = data;

    printf("[clients] requesting new xdg_toplevel\n");

	/* Allocate a tinywl_toplevel for this surface */
	struct wayterra_toplevel *toplevel = calloc(1, sizeof(*toplevel));
	toplevel->server = server;
	toplevel->xdg_toplevel = xdg_toplevel;
	toplevel->scene_tree =
		wlr_scene_xdg_surface_create(&toplevel->server->scene->tree, xdg_toplevel->base);
	toplevel->scene_tree->node.data = toplevel;
	xdg_toplevel->base->data = toplevel->scene_tree;

	/* Listen to the various events it can emit */
    toplevel->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
	toplevel->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
	toplevel->commit.notify = xdg_toplevel_commit;
	wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);
	//
	toplevel->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);
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


void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
	/* Called when the xdg_toplevel is destroyed. */
	struct wayterra_toplevel *toplevel = wl_container_of(listener, toplevel, destroy);

	wl_list_remove(&toplevel->map.link);
	wl_list_remove(&toplevel->unmap.link);
	wl_list_remove(&toplevel->commit.link);
	wl_list_remove(&toplevel->destroy.link);
	// wl_list_remove(&toplevel->request_move.link);
	// wl_list_remove(&toplevel->request_resize.link);
	// wl_list_remove(&toplevel->request_maximize.link);
	// wl_list_remove(&toplevel->request_fullscreen.link);

	free(toplevel);
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
