#include <wayland-server-core.h>
#include "server.h"


void focus_toplevel(struct wayterra_toplevel *toplevel);
void begin_interactive(struct wayterra_toplevel *toplevel, enum wayterra_cursor_mode mode, uint32_t edges);


void xdg_toplevel_request_resize( struct wl_listener *listener, void *data);
void xdg_toplevel_request_move( struct wl_listener *listener, void *data);
void xdg_toplevel_destroy(struct wl_listener *listener, void *data);
void xdg_toplevel_commit(struct wl_listener *listener, void *data);
void xdg_toplevel_map(struct wl_listener *listener, void *data);
void xdg_toplevel_unmap(struct wl_listener *listener, void *data);
void server_new_xdg_toplevel(struct wl_listener *listener, void *data);
void server_new_xdg_popup(struct wl_listener *listener, void *data);

void server_new_xdg_decoration( struct wl_listener *listener, void *data);
